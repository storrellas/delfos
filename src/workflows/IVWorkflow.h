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
#include <wcore/model/config/elements/Service.h>
#include <wcore/model/session/elements/Lead.h>
#include <wcore/constants/lm_constants.h>
#include <wcore/utils/unit_test/tester/PBXTester.h>

#include <workflows/Workflow.h>

namespace delfos {
namespace enzo {

class IVWorkflow: public Workflow {
private:
  // Common variables between sequence and alive thread
  std::mutex _session_mutex;
  std::string _session_guid;
  std::string _lead_guid;
  std::string _pin;

  // Stores the phone of the customer for this lead
  std::string _lead_customer_phone;

  /**
   * Stores call status
   */
  delfos::core::model::Call _call;
  std::atomic<bool> _call_bridged;

  /**
   * Generates requests for PBX
   */
  TwilioMockup* _twilio_mockup;

  /**
   * Thread to send alive request
   * @param alive_can_continue
   * @param agent_guid
   * @param session_guid
   * @param lead_guid
   */
  void alive_worker(const std::atomic<bool>& alive_can_continue);

public:
  constexpr static const char* IV_WORKFLOW = "iv";
  static const string get_workflow_name(){ return string(IV_WORKFLOW); }
  IVWorkflow(
      /** workflow base class ctor parameters **/
      const std::atomic<bool>&,
      Barrier&,
      const delfos::core::model::WizardBucket&,
      delfosErrorHandler* weh,
      RequestTraceNodes* request_traces,
      uint agent_position,
      TwilioMockup* twilio_mockup);
  virtual ~IVWorkflow();

  /**
   * Defines the procedure to be run for this workflow
   */
  void sequence() override;

  /**
   * Raised when an event from twilio was received
   * @param http_request
   */
  bool twilio_event(const WHttp http_request) override;

  /**
   * Raised when an event from twilio was generated
   * @param http_request
   */
  bool twilio_request(const WHttp http_request, const WHttp http_response);
};


} /* namespace enzo */
} /* namespace delfos */
