/*
 * AgentWorkflow.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: mjlopez
 */



#include "AgentWorkflow.h"


namespace wc = delfos::core;
namespace wcm = delfos::core::model;
namespace we = delfos::enzo;

namespace {

const wc::void_event_type undefined = wc::void_event_type::undefined;

} /// namespace

we::AgentWorkflow::AgentWorkflow(
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
    _lead_guid()
{
  set_workflow_name(AGENT_WORKFLOW);
}

we::AgentWorkflow::~AgentWorkflow() {}

void we::AgentWorkflow::sequence() {
  BLOG_TRACE_STARTED();
  _workflow_id = syscall(SYS_gettid);
  _workflow_running.store(true);
  try{

    _barrier.increment();

    const std::string agent_guid = get_agent_guid(_agent_position);
    const std::set<std::string> agent_vateams = get_vateam_guids();
    const std::set<std::string> agent_vacenters = get_vacenter_guids();

    /// Alive - Desktop
    std::atomic<bool> alive_can_continue(true);
    std::thread alive_thread( &AgentWorkflow::alive_worker, this, std::ref(alive_can_continue));

    while (true) {
      if (not _process_can_continue.load()) {
        BLOG_TRACE_FINISHED();
        break;
      }

      /// Getvateams - Desktop
      sleep_delta();
      WJson json_response;
      bool request_succeeded = true;
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

      /// change status
      init_trace(wc::lm_event_type::desktop_changestatus_agent);
      request_succeeded =
          _lm_tester.request_change_status(
              agent_guid,
              local_session_guid,
              wc::agent_status::available,
              json_response);
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
        init_trace(wc::lm_event_type::desktop_changestatus_agent);
        request_succeeded =
            _lm_tester.request_check_vateam(
                agent_guid,
                local_session_guid,
                vateam,
                json_response);
        end_trace(request_succeeded, json_response);
        if (!request_succeeded) {
          BLOG_FATAL(
            LogItem( wc::lm_event_type::desktop_check_vateam,
                wc::concat( "failed for agent '", agent_guid, "'", json_response.to_str())));
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
        LogItem(
            wc::lm_event_type::desktop_logout_agent,
          wc::concat("request_logout ok for agent '", agent_guid, "'")));
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
we::AgentWorkflow::alive_worker(
    const std::atomic<bool>& alive_can_continue) {

  BLOG_TRACE_STARTED();

  std::unique_lock<std::mutex> session_lock;

  try{

    string session_guid_local, agent_guid_local, lead_guid_local;
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


      /// LM : Alive - Desktop
      alive_lm(agent_guid_local, session_guid_local, lead_guid_local);

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


  } catch (const std::exception& e) {
      BLOG_FATAL(
        LogItem(undefined, wc::concat("std::exception caught: ", e.what())));
      session_lock.unlock();
      return;
  }

}

