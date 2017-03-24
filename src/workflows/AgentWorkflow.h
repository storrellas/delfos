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

#include <workflows/Workflow.h>

namespace delfos {
namespace enzo {

class AgentWorkflow: public Workflow {
private:

  // Common variables between sequence and alive thread
  std::mutex _session_mutex;
  std::string _session_guid;
  std::string _lead_guid;

public:
  constexpr static const char* AGENT_WORKFLOW = "agent";
  static const string get_workflow_name(){ return string(AGENT_WORKFLOW); }
  AgentWorkflow(
      /** workflow base class ctor parameters **/
      const std::atomic<bool>&,
      Barrier&,
      const delfos::core::model::WizardBucket&,
      delfosErrorHandler* weh,
      RequestTraceNodes* request_traces,
      uint agent_position);
  virtual ~AgentWorkflow();

private:

  /**
   * Defines the procedure to be run for this workflow
   */
  void sequence() override;

  /**
   * Thread to send alive request
   * @param alive_can_continue
   * @param agent_guid
   * @param session_guid
   * @param lead_guid
   */
  void alive_worker(const std::atomic<bool>& alive_can_continue);

  /**
   * Twilio Event within Agent Workflow
   * @param
   * @return
   */
  bool twilio_event(const WHttp){
    BLOG_TRACE(LogItem(core::void_event_type::undefined, "CALL METHOD FOR AGENTWORKFLOW IS USELESS"));
    return true;
  };

};

void
workflow_worker();

} /* namespace enzo */
} /* namespace delfos */
