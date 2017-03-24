/*
 * AgentWorkflow.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: mjlopez
 */



#include "delfosnarWorkflow.h"


namespace wc = delfos::core;
namespace wcm = delfos::core::model;
namespace we = delfos::enzo;
namespace wcct = delfos::core::constants::tags;

namespace {

const wc::void_event_type undefined = wc::void_event_type::undefined;

} /// namespace

we::delfosnarWorkflow::delfosnarWorkflow(
    /** workflow base class ctor parameters **/
    const std::atomic<bool>& process_can_continue,
    Barrier& barrier,
    const delfos::core::model::WizardBucket& wizard_bucket,
    delfosErrorHandler* weh,
    RequestTraceNodes* request_traces,
    uint agent_position) :
    Workflow( process_can_continue, barrier, wizard_bucket, weh,
              request_traces, agent_position),
    _session_mutex(),
    _session_guid(),
    _delfosnar_guid(),
    _gari_viewer_guid(0),
    _gari_presenter_guid(0)
{
  set_workflow_name(WHISBINAR_WORKFLOW);
}

we::delfosnarWorkflow::~delfosnarWorkflow() {}


//
// Workflow sequence
//

void we::delfosnarWorkflow::sequence() {
  BLOG_TRACE_STARTED();
  _workflow_id = syscall(SYS_gettid);
  _workflow_running.store(true);

  try{

    bool request_succeeded = true;
    WJson json_response;

    // Wait for all sequence to be started
    _barrier.increment();

    const std::string agent_guid = get_agent_guid(_agent_position);
    const std::set<std::string> agent_vateams = get_vateam_guids();
    const std::set<std::string> agent_vacenters = get_vacenter_guids();
    const std::string branchgroup_guid = get_branchgroup_delfosnar_guid(_agent_position);
    const std::string branch_guid = get_branch_delfosnar_guid(_agent_position);

    /// Generate seed
    gari_create_seed();

    /// Alive - Desktop
    std::atomic<bool> alive_can_continue(true);
    std::thread alive_thread( &delfosnarWorkflow::alive_worker, this, std::ref(alive_can_continue));

    while (true) {
      if (not _process_can_continue.load()) {
        BLOG_TRACE_FINISHED();
        break;
      }

      /// Getvateams - Desktop
      sleep_delta();
      for (const auto& vacenter : agent_vacenters) {
        init_trace(wc::lm_event_type::desktop_getvateams_agent);
        request_succeeded =
            _lm_tester.request_getvateams(agent_guid, vacenter, json_response);
        end_trace(request_succeeded, json_response);
        if (!request_succeeded) {
          BLOG_FATAL(
            LogItem( wc::lm_event_type::desktop_getvateams_agent,
                      wc::concat( "failed for agent '", agent_guid, "'")));
          break;
        }
      }

      sleep_delta();
      if (not _process_can_continue.load()){
        BLOG_TRACE_FINISHED();
        break;
      }


      /// Login - Desktop
      init_trace(wc::lm_event_type::desktop_login_agent);
      request_succeeded = _lm_tester.request_login(agent_guid, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(LogItem(wc::lm_event_type::desktop_login_agent,
                              wc::concat("failed for agent '", agent_guid, "'")));
        break;
      }
      // Capture session_id from login
      if (not json_response.HasMember(WAGENT_KEY_STR_SESSION_ID) or
          not json_response[WAGENT_KEY_STR_SESSION_ID].IsString()){
        BLOG_FATAL(
          LogItem(wc::lm_event_type::desktop_login_agent, wc::concat( "failed for agent '", agent_guid, "', ",
                            "session_guid is not present in response, json: '", json_response.to_str(), "'")));
        break;
      }
      std::unique_lock<std::mutex> session_lock(_session_mutex);
      _session_guid = json_response[WAGENT_KEY_STR_SESSION_ID].GetString();
      string local_session_guid = _session_guid;
      session_lock.unlock();

      sleep_delta();
      if (not _process_can_continue.load()){
        BLOG_TRACE_FINISHED();
        break;
      }

      /// Change status - Desktop
      init_trace(wc::lm_event_type::desktop_changestatus_agent);
      request_succeeded =
          _lm_tester.request_change_status( agent_guid, local_session_guid,
                                              wc::agent_status::available, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(
          LogItem( wc::lm_event_type::desktop_changestatus_agent, wc::concat( "failed for agent '", agent_guid, "'")));
        break;
      }

      sleep_delta();
      if (not _process_can_continue.load()){
        BLOG_TRACE_FINISHED();
        break;
      }

      /// Check_vateam - Desktop
      bool check_vateam_ok = true;
      for (const auto& vateam : agent_vateams) {
        init_trace(wc::lm_event_type::desktop_check_vateam);
        request_succeeded =
            _lm_tester.request_check_vateam( agent_guid, local_session_guid,
                                            vateam, json_response);
        end_trace(request_succeeded, json_response);
        if (!request_succeeded) {
          BLOG_FATAL(
            LogItem( wc::lm_event_type::desktop_check_vateam,
                wc::concat( "failed for agent '", agent_guid, "' ", json_response.to_str())));
          check_vateam_ok = false;
          break;
        }
      }
      // If the command failed we should abort operation
      if( not check_vateam_ok ){
        BLOG_TRACE_FINISHED();
        break;
      }

      /// Get availability - Landing
      init_trace(wc::lm_event_type::webuser_get_availability);
      request_succeeded = _lm_tester.request_get_availability(branch_guid, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(
          LogItem( wc::lm_event_type::webuser_get_availability,
                    wc::concat( "failed for agent '", agent_guid, "'")));
        break;
      }

      sleep_delta();
      if (not _process_can_continue.load()){
        BLOG_TRACE_FINISHED();
        break;
      }

      /// delfosnar Alive - Desktop
      set<string> branchgroup_set;
      branchgroup_set.insert(branchgroup_guid);
      init_trace(wc::lm_event_type::desktop_alive);
      request_succeeded = _lm_tester.request_alive(agent_guid, _session_guid, branchgroup_set, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(
          LogItem( wc::lm_event_type::desktop_alive,
                    wc::concat("failed for agent '", agent_guid, "'", json_response.to_str())));
        throw wc::Exception( as_function(__PRETTY_FUNCTION__) + ": " + as_error("not working"));
      }

      sleep_delta();
      if (not _process_can_continue.load()){
        break;
      }

      /// Create delfosnar - Desktop
      init_trace(wc::lm_event_type::desktop_create);
      request_succeeded = _lm_tester.request_create_delfosnar(branchgroup_guid, WHISBINAR_TITLE, WHISBINAR_DESCRIPTION,
                                                              _seed_guid, agent_guid, local_session_guid, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(
          LogItem( wc::lm_event_type::desktop_create,
                    wc::concat( "failed for agent '", agent_guid, "'", json_response.to_str())));
        break;
      }
      if(json_response.HasMember(WHISBINAR_KEY_STR_CLASS)
          and json_response[WHISBINAR_KEY_STR_CLASS].IsObject()){

         WJson delfosnar_json = json_response[WHISBINAR_KEY_STR_CLASS];
         if(delfosnar_json.HasMember(wcct::guid)
             and delfosnar_json[wcct::guid].IsString()){
            _delfosnar_guid = delfosnar_json[wcct::guid].GetString();
         }
      }

      sleep_delta();
      if (not _process_can_continue.load()){
        break;
      }

      /// Create Presenter - Gari Read Room
      gari_create_user(wc::room_user_type::presenter, _gari_presenter_guid);

      sleep_delta();
      if (not _process_can_continue.load())
        break;

      /// Create Viewer - Gari Read Room
      gari_create_user(wc::room_user_type::viewer, _gari_viewer_guid);

      sleep_delta();
      if (not _process_can_continue.load())
        break;

      /// Several room updates (both viewer, presenter)
      bool res = true;
      for(int i = 0; i < WHISBINAR_UPDATE_ROOM; ++i){

        // Gari Update room - Presenter
        if(!gari_update_room(_gari_presenter_guid)){
          res = false;
          break;
        }

        sleep_delta();
        if (not _process_can_continue.load()){
          res = false;
          break;
        }

        // Gari Update room - Viewer
        if(!gari_update_room(_gari_viewer_guid)){
          res = false;
          break;
        }

        sleep_delta();
        if (not _process_can_continue.load()){
          res = false;
          break;
        }

      }
      if( !res ) break;

      /// Close delfosnar - Desktop
      init_trace(wc::lm_event_type::desktop_close);
      request_succeeded = _lm_tester.request_close(_delfosnar_guid, agent_guid, local_session_guid, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(
          LogItem( wc::lm_event_type::desktop_close,
                    wc::concat( "failed for agent '", agent_guid, "'", json_response.to_str())));
        break;
      }

      // clear session_guid for alive worker
      session_lock.lock();
      _session_guid.clear();
      _delfosnar_guid.clear();
      _gari_viewer_guid = 0;
      _gari_presenter_guid = 0;
      session_lock.unlock();
      /// Logout - Desktop
      init_trace(wc::lm_event_type::desktop_logout_agent);
      request_succeeded =
          _lm_tester.request_logout(agent_guid, local_session_guid, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(
          LogItem( wc::lm_event_type::desktop_logout_agent, wc::concat("failed for agent '", agent_guid, "'")));
        break;
      }

      BLOG_TRACE(
        LogItem(undefined, wc::concat("request_logout ok for agent '", agent_guid, "'")));

//      // ------------------------
//      cout << "++++++++++++++++++++++++++" << endl;
//      cout << "Single iteration enabled" << endl;
//      cout << "++++++++++++++++++++++++++" << endl;
//      break;
//      // ------------------------

    }
    /// stop alives
    alive_can_continue.store(false);
    if (alive_thread.joinable())
      alive_thread.join();

  } catch (const std::exception& e) {
      BLOG_FATAL(
        LogItem(undefined, wc::concat("std::exception caught: ", e.what())));
      return;
  }

  _workflow_running.store(false);
  BLOG_TRACE_FINISHED();
}

bool we::delfosnarWorkflow::gari_update_room(uint gari_user_guid){

  WJson json_response;
  wcm::ChatFilePod file_pod = generate_chat_file_pod(gari_user_guid);
  init_trace(wc::gari_event_type::update_room);
  bool request_succeeded =
      _gari_tester.request_update(_delfosnar_guid,gari_user_guid,
                                  WORKFLOW_CHATFILE_POD,
                                  &file_pod, json_response);
  end_trace(request_succeeded, json_response);
  if (!request_succeeded) {
    BLOG_FATAL(
      LogItem( wc::gari_event_type::update_room, wc::concat("failed ", json_response.to_str())));
    return false;
  }
  return true;
}

bool we::delfosnarWorkflow::gari_create_user(delfos::core::room_user_type room_user_type, uint& gari_user_guid){

  WJson json_response;
  init_trace(wc::gari_event_type::read_room);
  bool request_succeeded =
      _gari_tester.request_read_session(_delfosnar_guid, room_user_type, json_response);
  end_trace(request_succeeded, json_response);
  if (!request_succeeded) {
    BLOG_FATAL(
      LogItem( wc::gari_event_type::read_room, wc::concat("failed ", json_response.to_str())));
    return false;
  }
  // Store user_id
  std::unique_lock<std::mutex> session_lock(_session_mutex);
  if(json_response.HasMember(wcm::ROOM_KEY_STR_USER_ID) and
    json_response[wcm::ROOM_KEY_STR_USER_ID].IsUint() ){
    gari_user_guid = json_response[wcm::ROOM_KEY_STR_USER_ID].GetUint();
  }
  session_lock.unlock();
  return true;
}

void we::delfosnarWorkflow::gari_read_room(const uint& gari_user_guid){
  WJson json_response;
  init_trace_alive(wc::gari_event_type::read_room);
  bool request_succeeded =
      _gari_tester.request_read_session_user(_delfosnar_guid, gari_user_guid, json_response);
  end_trace_alive(request_succeeded, json_response);
  if (!request_succeeded) {
    BLOG_FATAL(
      LogItem( wc::gari_event_type::read_room, wc::concat("failed", json_response.to_str())));
    throw wc::Exception( as_function(__PRETTY_FUNCTION__) + ": " + as_error("not working"));
  }
}

void
we::delfosnarWorkflow::alive_worker(const std::atomic<bool>& alive_can_continue) {
  BLOG_TRACE_STARTED();

  std::unique_lock<std::mutex> session_lock;

  try{

    string session_guid_local, seed_guid_local;
    const string agent_guid_local = get_agent_guid(_agent_position);

    while (true) {

      if (not alive_can_continue.load()) {
        BLOG_TRACE_FINISHED();
        return;
      }

      // Check whether user is logged in
      session_lock = std::unique_lock<std::mutex>(_session_mutex);
      if( _session_guid.empty() ){
        session_lock.unlock();
        std::this_thread::sleep_for(_polling_check_period);
        continue;
      }

      /// GARI : Read Session - Presenter
      if( not _delfosnar_guid.empty() and _gari_presenter_guid != 0){
        gari_read_room(_gari_presenter_guid);
      }

      /// GARI : Read Session - Viewer
      if( not _delfosnar_guid.empty() and _gari_viewer_guid != 0){
        gari_read_room(_gari_viewer_guid);
      }

      session_lock.unlock();

      if (not alive_can_continue.load()) {
        BLOG_TRACE_FINISHED();
        return;
      }
      /// wait for the next alive request
      std::chrono::milliseconds delta(_periodic_delta_const);
      BLOG_TRACE(
        LogItem(
          undefined,
          wc::concat("sleeping for delta '", delta.count(), "' ms")));
      std::this_thread::sleep_for(delta);

    }

  }catch (const std::exception& e) {
      BLOG_FATAL(
        LogItem(undefined, wc::concat("std::exception caught: ", e.what())));
      session_lock.unlock();
      return;
  }

}


