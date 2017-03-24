/*
 * AgentWorkflow.h
 *
 *  Created on: Feb 20, 2017
 *      Author: mjlopez
 */

#pragma once

#include <chrono>
#include <set>
#include <string>
#include <thread>

#include <wcore/logging/Logger.h>
#include <wcore/utils/unit_test/mockups/TwilioMockup.h>
#include <wcore/model/config/elements/Call.h>

#include <workflows/Workflow.h>

namespace delfos {
namespace enzo {

class CMNWorkflow: public Workflow {
private:

  // Common variables between sequence and alive thread
  std::mutex _session_mutex;
  std::string _session_guid;
  std::string _lead_guid;

  // Stores the phone of the customer for this lead
  std::string _lead_customer_phone;

  /**
   * Stores call status
   */
  delfos::core::model::Call _call;
  std::atomic<bool> _call_bridged;

  /**
   * Thread to send alive request
   * @param alive_can_continue
   * @param agent_guid
   * @param session_guid
   * @param lead_guid
   */
  void alive_worker(const std::atomic<bool>& alive_can_continue);


public:
  constexpr static const char* CMN_WORKFLOW = "cmn";
  static const string get_workflow_name(){ return string(CMN_WORKFLOW); }
  CMNWorkflow(
      /** workflow base class ctor parameters **/
      const std::atomic<bool>&,
      Barrier&,
      const delfos::core::model::WizardBucket&,
      delfosErrorHandler* weh,
      RequestTraceNodes* request_traces,
      uint agent_position);
  virtual ~CMNWorkflow();

  /**
   * Defines the procedure to be run for this workflow
   */
  void sequence() override;

  /**
   * Raised when an event from twilio was received
   * @param http_request
   */
  bool twilio_event(const WHttp http_request) override;
};


} /* namespace enzo */
} /* namespace delfos */
