/*
 * AgentWorkflow.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: mjlopez
 */



#include "ExpiredWorkflow.h"


namespace wc = delfos::core;
namespace wcm = delfos::core::model;
namespace we = delfos::enzo;

namespace {

const wc::void_event_type undefined = wc::void_event_type::undefined;

} /// namespace

const std::chrono::seconds
we::ExpiredWorkflow::_expired_iteration_polling = std::chrono::seconds(3); // 3 seconds of polling
const std::chrono::seconds
we::ExpiredWorkflow::_expired_iteration_period = std::chrono::seconds(60); // 30 minutes

we::ExpiredWorkflow::ExpiredWorkflow(
    /** workflow base class ctor parameters **/
    const std::atomic<bool>& process_can_continue,
    Barrier& barrier,
    const delfos::core::model::WizardBucket& wizard_bucket,
    delfosErrorHandler* weh,
    RequestTraceNodes* request_traces,
    uint agent_position) :
    Workflow( process_can_continue, barrier, wizard_bucket, weh,
              request_traces, agent_position),
    _lead_customer_phone()
{
  set_workflow_name(EXPIRED_WORKFLOW);
}

we::ExpiredWorkflow::~ExpiredWorkflow() {}


//
// Workflow sequence
//

void we::ExpiredWorkflow::sequence() {
  BLOG_TRACE_STARTED();
  _workflow_id = syscall(SYS_gettid);
  _workflow_running.store(true);

  try{

    bool request_succeeded = true;
    WJson json_response;

    // Wait for all sequence to be started
    _barrier.increment();

    const std::string agent_guid = get_agent_guid(_agent_position);
    const string branch_id = get_branch_guid();

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
      request_succeeded = _lm_tester.request_lead(wc::lead_action_type::scheduled_from_landing, branch_id,
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

//      // ------------------------
//      cout << "++++++++++++++++++++++++++" << endl;
//      cout << "Single iteration enabled" << endl;
//      cout << "++++++++++++++++++++++++++" << endl;
//      break;
//      // ------------------------

      BLOG_TRACE(
        LogItem(
          undefined,
          wc::concat("sleeping for delta '", _expired_iteration_period.count(), "' minutes")));

      // Wait for some time before proceeding
      const system_clock::time_point init_time = system_clock::now();
      while( true ){

        if (not _process_can_continue.load())
          break;


        if ( (system_clock::now() -  init_time) > _expired_iteration_period ) break;
        std::this_thread::sleep_for(_expired_iteration_polling);
      }

    }

  } catch (const std::exception& e) {
      BLOG_FATAL(
        LogItem(undefined, wc::concat("std::exception caught: ", e.what())));
      return;
  }

  _workflow_running.store(false);
  BLOG_TRACE_FINISHED();
}

