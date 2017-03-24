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

class delfosnarWorkflow: public Workflow {
private:

  // Common variables between sequence and alive thread
  std::mutex _session_mutex;
  std::string _session_guid;
  std::string _delfosnar_guid;
  uint _gari_viewer_guid;
  uint _gari_presenter_guid;

  /**
   * Thread to send alive request
   * @param alive_can_continue
   * @param agent_guid
   * @param session_guid
   * @param lead_guid
   */
  void alive_worker(const std::atomic<bool>& alive_can_continue);

  /**
   * Performs a gari update with the given user guid
   * @param gari_user_guid
   */
  bool gari_update_room(uint gari_user_guid);

  /**
   * Performs a gari update with the given user guid
   * @param gari_user_guid
   */
  bool gari_create_user(delfos::core::room_user_type room_user_type, uint& gari_user_guid);

  /**
   * Performs a gari read of the room
   * @param gari_user_guid
   */
  void gari_read_room(const uint& gari_user_guid);

public:
  constexpr static const char* WHISBINAR_WORKFLOW = "delfosnar";
  constexpr static const char* WHISBINAR_TITLE = "my-title";
  constexpr static const char* WHISBINAR_DESCRIPTION = "my-description";
  constexpr static int WHISBINAR_UPDATE_ROOM = 5; // Number of updates of the room before dying
  static const string get_workflow_name(){ return string(WHISBINAR_WORKFLOW); }
  delfosnarWorkflow(
      /** workflow base class ctor parameters **/
      const std::atomic<bool>&,
      Barrier&,
      const delfos::core::model::WizardBucket&,
      delfosErrorHandler* weh,
      RequestTraceNodes* request_traces,
      uint agent_position);
  virtual ~delfosnarWorkflow();

  /**
   * Defines the procedure to be run for this workflow
   */
  void sequence() override;

};


} /* namespace enzo */
} /* namespace delfos */
