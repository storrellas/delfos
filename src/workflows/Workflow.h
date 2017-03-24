/*
 * Workflow.h
 *
 *  Created on: Feb 20, 2017
 *      Author: mjlopez
 */

#pragma once

#include <wcore/model/config/elements/WizardBucket.h>
#include <wcore/model/config/elements/Room.h>
#include <wcore/utils/unit_test/tester/LMTester.h>
#include <wcore/utils/unit_test/tester/GARITester.h>

#include <src/model/session/containers/RequestTraceNodes.h>
#include <src/EnzoConfiguration.h>
#include <src/utils/Barrier.h>

namespace delfos {
namespace enzo {

class Workflow {
public:
  Workflow(
      const std::atomic<bool>&,
      Barrier&,
      const delfos::core::model::WizardBucket&,
      delfosErrorHandler* weh,
      RequestTraceNodes* request_traces,
      uint agent_position = 0);
  virtual ~Workflow();

  void start();
  void terminate();
  virtual bool twilio_event(const WHttp){
    BLOG_TRACE(LogItem(core::void_event_type::undefined, "CALL METHOD NOT ALLOWED"));
    return false;
  };
  virtual bool twilio_request(const WHttp, const WHttp){
    BLOG_TRACE(LogItem(core::void_event_type::undefined, "CALL METHOD NOT ALLOWED"));
    return false;
  }

//  virtual void sequence() = 0;
  virtual void sequence(){
    BLOG_FATAL(LogItem(core::void_event_type::undefined, "CALL METHOD NOT ALLOWED"));
  };
  void sleep_delta();

  /**
   * Returns whether workflow is running
   * @return
   */
  bool get_workflow_running(){ return _workflow_running.load(); }

private:
  virtual void _join();
protected:
  const static std::chrono::milliseconds _polling_check_period;
  const static std::chrono::milliseconds _polling_check_max;

  constexpr static const char* WORKFLOW_PHONE = "+34659652236";
  constexpr static const char* WORKFLOW_PHONE_COUNTRY = "ES";
  constexpr static const char* WORKFLOW_NAME = "my-name";
  constexpr static const char* WORKFLOW_MY_EMAIL = "user@mail.com";
  constexpr static uint WORKFLOW_LEAD_RESULT = 1007;


  constexpr static int WORKFLOW_CHATFILE_POD = 0;
  constexpr static int WORKFLOW_CAMERA_POD = 1;
  constexpr static int WORKFLOW_SHARING_POD = 2;

  // General logger
  delfosErrorHandler* _weh;

  // Testers
  LMTester _lm_tester;
  GARITester _gari_tester;

  // Stores the current user guid for gari
  std::string _seed_guid;
  uint _gari_user_guid;

  // Random number generator
  std::mt19937 _random_generator;
  std::uniform_int_distribution<int> _unif_dist;

  /**
   * Variables to generate random phone
   */
  std::uniform_int_distribution<int> _phone_uniform_dist;
  std::random_device _random_device;


  // Periodic delta (fixed value
  std::chrono::milliseconds _periodic_delta_const;

  // Used to launch sequence
  std::thread _thread;

  // Agent position in Wizard Bucket
  uint _agent_position;

  // Wizard Bucket
  const delfos::core::model::WizardBucket& _wizard_bucket;

  // Atomic to abort sequence
  const std::atomic<bool>& _process_can_continue;

  // Atomic to check whether workflow is running
  std::atomic<bool> _workflow_running;

  // Used to wait for all workflows to start
  Barrier& _barrier;

  // RequestTrace
  RequestTraceNodes* _request_traces;
  delfos::core::model::RequestTrace _trace;
  delfos::core::model::RequestTrace _trace_alive;
  std::string _workflow_name;
  uint _workflow_id;

  // Generates Random phone
  const std::string generate_random_phone();

  // Generates chat message to update pod
  const delfos::core::model::ChatFilePod generate_chat_file_pod(unsigned int source_id);

  /// Request Trace methods
  void init_trace(const delfos::core::lm_event_type event);
  void init_trace(const delfos::core::gari_event_type event);
  void end_trace(const bool& request_success, const WJson& json_response);
  void init_trace_alive(const delfos::core::lm_event_type event);
  void init_trace_alive(const delfos::core::gari_event_type event);
  void end_trace_alive(const bool& request_success, const WJson& json_response);

  // Configure Workflow
  void set_agent_position( const uint& value ){ _agent_position = value; }
  const uint& get_agent_position(){ return _agent_position; }
  void set_workflow_name(const std::string& value){ _workflow_name = value; }
  const std::string& get_workflow_name(){ return _workflow_name; }

  // Get items from WizardBucket
  const std::string get_agent_guid( uint agent_position);
  const std::set<std::string> get_vateam_guids();
  const std::set<std::string> get_vacenter_guids();
  const std::string get_branch_guid();
  const std::string get_branchgroup_delfosnar_guid( uint branchgroup_position );
  const std::string get_branch_delfosnar_guid( uint branch_position );


  // Generates seed to GARI
  void gari_create_seed();
  bool gari_update_room(const std::string& session_id);

  // Generates alive for LM
  void alive_lm(const std::string& agent_guid,
                const std::string& session_guid,
                const std::string& lead_guid);

  // Generates an alive for gari
  void alive_gari(const std::string& lead_guid);

};

} /* namespace enzo */
} /* namespace delfos */
