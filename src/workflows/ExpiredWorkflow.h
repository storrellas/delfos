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

class ExpiredWorkflow: public Workflow {
private:

  const static std::chrono::seconds _expired_iteration_polling;
  const static std::chrono::seconds _expired_iteration_period;

  // Stores the phone of the customer for this lead
  std::string _lead_customer_phone;




public:

  constexpr static const char* EXPIRED_WORKFLOW = "expired";
  static const string get_workflow_name(){ return string(EXPIRED_WORKFLOW); }
  ExpiredWorkflow(
      /** workflow base class ctor parameters **/
      const std::atomic<bool>&,
      Barrier&,
      const delfos::core::model::WizardBucket&,
      delfosErrorHandler* weh,
      RequestTraceNodes* request_traces,
      uint agent_position);
  virtual ~ExpiredWorkflow();

  /**
   * Defines the procedure to be run for this workflow
   */
  void sequence() override;

};


} /* namespace enzo */
} /* namespace delfos */
