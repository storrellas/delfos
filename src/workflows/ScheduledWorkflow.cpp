/*
 * AgentWorkflow.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: mjlopez
 */



#include "ScheduledWorkflow.h"


namespace wc = delfos::core;
namespace wcm = delfos::core::model;
namespace we = delfos::enzo;

namespace {

const wc::void_event_type undefined = wc::void_event_type::undefined;

} /// namespace

we::ScheduledWorkflow::ScheduledWorkflow(
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
    _lead_guid(),
    _lead_customer_phone(),
    _call(),
    _call_bridged(false)
{
  set_workflow_name(SCHEDULED_WORKFLOW);
}

we::ScheduledWorkflow::~ScheduledWorkflow() {}


//
// Workflow sequence
//

void we::ScheduledWorkflow::sequence() {
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
    const string branch_id = get_branch_guid();

    /// Generate seed
    gari_create_seed();

    /// Alive - Desktop
    std::atomic<bool> alive_can_continue(true);
    std::thread alive_thread( &ScheduledWorkflow::alive_worker, this, std::ref(alive_can_continue));

    while (true) {
      if (not _process_can_continue.load()) {
        BLOG_TRACE_FINISHED();
        break;
      }

      /// Schedule Lead - Landing
      sleep_delta();
      wcm::SessionMetadata metadata;
      metadata.fillTestMetadata(NewGuid());
      _lead_customer_phone = generate_random_phone();
      std::chrono::system_clock::time_point scheduled_datetime =
          std::chrono::system_clock::now();
      init_trace(wc::lm_event_type::webuser_request_lead);
      request_succeeded = _lm_tester.request_lead(wc::lead_action_type::call_me_back, branch_id,
                                                    _lead_customer_phone, WORKFLOW_PHONE_COUNTRY,
                                                    WORKFLOW_NAME, WORKFLOW_MY_EMAIL, _seed_guid,
                                                    scheduled_datetime, metadata, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(
          LogItem( wc::lm_event_type::webuser_request_lead,
                    wc::concat( "failed for agent '", agent_guid, "' ", json_response.to_str())));
        break;
      }

      sleep_delta();
      if (not _process_can_continue.load()){
        BLOG_TRACE_FINISHED();
        break;
      }

      /// Getvateams - Desktop
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

      sleep_delta();
      if (not _process_can_continue.load()){
        BLOG_TRACE_FINISHED();
        break;
      }

      /// Change status (busy) - Desktop
      init_trace(wc::lm_event_type::desktop_changestatus_agent);
      request_succeeded =
          _lm_tester.request_change_status( agent_guid, local_session_guid,
                                              wc::agent_status::busy, json_response);
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

      /// Change status (available) - Desktop
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

      /// Wait for scheduled lead to be launched
      _call_bridged.store(false);
      const system_clock::time_point init_time = system_clock::now();
      while( true ){
        if( _call_bridged.load() ) break;
        if ( (system_clock::now() -  init_time) > _polling_check_max ) break;
        std::this_thread::sleep_for(_polling_check_period);
      }
      if( not _call_bridged.load() ){
        BLOG_FATAL( LogItem(wc::lm_event_type::webuser_request_lead,
            wc::concat("Exceeded time for call bridged ", wc::as_db_string(init_time), " ",
                                                          wc::as_db_string(system_clock::now()))));
        break;
      }
      _call.clear();

      /// Open session - Desktop
      init_trace(wc::lm_event_type::desktop_open_session);
      request_succeeded = _lm_tester.request_open_session(_lead_customer_phone, agent_guid, _session_guid, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(
          LogItem( wc::lm_event_type::desktop_open_session, wc::concat( "failed for agent '", agent_guid, "'",
                                              " json: '", json_response.to_str(), "'")));
        break;
      }
      // Capture lead guid
      session_lock.lock();
      _lead_guid.clear();
      _lead_session_guid.clear();
      if(json_response.HasMember(LEAD_KEY_STR_LEAD.c_str())
          and json_response[LEAD_KEY_STR_LEAD.c_str()].IsObject()){

         WJson lead_json = json_response[LEAD_KEY_STR_LEAD.c_str()];
         if(lead_json.HasMember(LEAD_KEY_STR_GUID.c_str())
             and lead_json[LEAD_KEY_STR_GUID.c_str()].IsString()){
            _lead_guid = lead_json[LEAD_KEY_STR_GUID.c_str()].GetString();
         }
         if(lead_json.HasMember(LEAD_KEY_STR_SESSION.c_str())
             and lead_json[LEAD_KEY_STR_SESSION.c_str()].IsString()){
           _lead_session_guid = lead_json[LEAD_KEY_STR_SESSION.c_str()].GetString();
         }
      }
      if ( _lead_guid.empty() ){
        BLOG_FATAL(
          LogItem(wc::lm_event_type::webuser_request_lead, wc::concat( "failed for agent '", agent_guid, "' ",
                                          ", json: '", json_response.to_str(), "'")));
        break;
      }
      string local_lead_guid = _lead_guid;
      session_lock.unlock();


      sleep_delta();
      if (not _process_can_continue.load())
        break;

      /// Set Result - Desktop
      session_lock.lock();
      _lead_guid.clear();
      _lead_session_guid.clear();
      _gari_user_guid = 0;
      session_lock.unlock();
      init_trace(wc::lm_event_type::desktop_set_result);
      request_succeeded = _lm_tester.request_set_result(local_lead_guid, WORKFLOW_LEAD_RESULT, json_response);
      end_trace(request_succeeded, json_response);
      if (!request_succeeded) {
        BLOG_FATAL(
          LogItem( wc::lm_event_type::desktop_set_result, wc::concat( "failed for agent '", agent_guid, "'",
                                            " json: '", json_response.to_str(), "'")));
        break;
      }

      sleep_delta();
      if (not _process_can_continue.load()) break;

      // clear session_guid for alive worker
      session_lock.lock();
      _session_guid.clear();
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

void
we::ScheduledWorkflow::alive_worker(const std::atomic<bool>& alive_can_continue) {
  BLOG_TRACE_STARTED();

  std::unique_lock<std::mutex> session_lock;

  try{

    string session_guid_local, agent_guid_local, lead_guid_local, seed_guid_local;
    agent_guid_local = get_agent_guid(_agent_position);

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
      session_guid_local = _session_guid;
      lead_guid_local = _lead_guid;


      /// LM : Alive - Desktop
      alive_lm(agent_guid_local, session_guid_local, lead_guid_local);

      /// GARI : Read Session - Desktop
      if( not _lead_session_guid.empty() )
        alive_gari(_lead_session_guid);

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

//
// Handler for twilio event
//
bool we::ScheduledWorkflow::twilio_event(const WHttp http_request){

  const string url = http_request.get_url_param("url");
  wc::twilio_event_type event_type = TwilioMockup::get_twilio_event_type(http_request);
  if(event_type == wc::twilio_event_type::call){

    const string calling_to = http_request.get_url_param("to");
    const string calling_from = http_request.get_url_param("from");
    if( calling_to == _lead_customer_phone){
      // Check twilio event corresponds to our workflow - calling user

      size_t init_index = url.find("/ch-usr_") +1;
      size_t end_index = url.find("/calling_response")-1;
      const string session_url = url.substr(init_index +1, end_index - init_index);
      _call.get_user_channel().call();
      _call.get_user_channel().pbx_ok();
      _call.get_user_channel().set_twilio_session(session_url);
    }
    if( calling_from == _lead_customer_phone){
      // Check twilio event corresponds to our workflow - calling agent

      size_t init_index = url.find("/ch-agt_") +1;
      size_t end_index = url.find("/calling_response") -1 ;
      const string session_url = url.substr(init_index, end_index - init_index);

      _call.get_agent_channel().call();
      _call.get_agent_channel().pbx_ok();
      _call.get_agent_channel().set_twilio_session(session_url);
    }

  }else if(event_type == wc::twilio_event_type::bridge){

    const string twilio_user_session =
        _call.get_user_channel().get_twilio_session();
    const string twilio_agent_session =
        _call.get_agent_channel().get_twilio_session();
    if(url.find(twilio_user_session) != string::npos){
      _call.get_user_channel().bridge();
      _call.get_user_channel().pbx_ok();

    }else if(url.find(twilio_agent_session) != string::npos){
      _call.get_agent_channel().bridge();
      _call.get_agent_channel().pbx_ok();
    }

    if(_call.is_call_agent_user_bridged()){
      _call_bridged.store(true);
    }

  }else if(event_type == wc::twilio_event_type::hangup){
    _call_bridged.store(false); // This shouldnt be necessary
  }
  return true;
}
