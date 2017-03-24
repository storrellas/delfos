/*
 * Workflow.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: mjlopez
 */

#include "Workflow.h"

namespace we = delfos::enzo;
namespace wcm = delfos::core::model;

namespace {
const wc::void_event_type undefined = wc::void_event_type::undefined;
}

const std::chrono::milliseconds
we::Workflow::_polling_check_period = std::chrono::milliseconds(100);
const std::chrono::milliseconds
we::Workflow::_polling_check_max = std::chrono::milliseconds(20000);


we::Workflow::Workflow(
    const std::atomic<bool>& process_can_continue,
    Barrier& barrier,
    const delfos::core::model::WizardBucket& wizard_bucket,
    delfosErrorHandler* weh,
    RequestTraceNodes* request_traces,
    uint agent_position):
  _weh(weh),
  _lm_tester(weh),
  _gari_tester(weh),
  _seed_guid(),
  _gari_user_guid(0),
  _random_generator(std::random_device()()),
  _unif_dist(),
  _periodic_delta_const(),
  _thread(),
  _agent_position(agent_position),
  _wizard_bucket(wizard_bucket),
  _process_can_continue(process_can_continue),
  _workflow_running(false),
  _barrier(barrier),
  _request_traces(request_traces),
  _trace(),
  _trace_alive(),
  _workflow_name(),
  _workflow_id(0)
{
}

void we::Workflow::start(){
  EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
  const uint delta_min_uint = configuration->get_uint(EnzoConfiguration::KEY_STR_DELTA_MIN);
  const uint delta_max_uint = configuration->get_uint(EnzoConfiguration::KEY_STR_DELTA_MAX);
  const uint delta_periodic_uint = configuration->get_uint(EnzoConfiguration::KEY_STR_DELTA_PERIODIC);
  const std::chrono::milliseconds delta_min(delta_min_uint);
  const std::chrono::milliseconds delta_max(delta_max_uint);
  const std::chrono::milliseconds periodic_delta_const(delta_periodic_uint);
  const string lm_ip =  configuration->get_string(EnzoConfiguration::KEY_STR_LM_IP);
  const string lm_port =  configuration->get_string(EnzoConfiguration::KEY_STR_LM_PORT_HTTP);
  const string gari_ip =  configuration->get_string(EnzoConfiguration::KEY_STR_GARI_IP);
  const string gari_port =  configuration->get_string(EnzoConfiguration::KEY_STR_GARI_PORT_HTTP);

  _periodic_delta_const = periodic_delta_const;
  _unif_dist = std::uniform_int_distribution<int>(delta_min.count(), delta_max.count());
  _lm_tester.set_web_connection_parameters(lm_ip, lm_port);
  _lm_tester.init(false, false);  /// mockups disabled (hardcoded)
  _gari_tester.set_web_connection_parameters(gari_ip, gari_port);
  _gari_tester.init(false, false);  /// mockups disabled (hardcoded)

  _thread = std::thread(&Workflow::sequence, this);
}

void we::Workflow::terminate(){
  _join();
}

we::Workflow::~Workflow() {
  _join();
}

void we::Workflow::_join() {
  if (_thread.joinable())
    _thread.join();
}

const std::string we::Workflow::generate_random_phone(){
  string phone = WORKFLOW_PHONE;
  phone = phone.substr(0, phone.size() - 6 );

  // Generate random phone
  string random_phone = phone + std::to_string(_phone_uniform_dist(_random_generator));
  random_phone.resize(12, '0');
  return random_phone;
}

void we::Workflow::sleep_delta(){
  std::chrono::milliseconds delta(_unif_dist(_random_generator));
  BLOG_TRACE(
    LogItem(
       undefined,
       wc::concat("sleeping for delta '", delta.count(), "' ms")));
  std::this_thread::sleep_for(delta);
}

// ------------------
// WizardBucket methods
// ------------------

const std::string
we::Workflow::get_agent_guid(uint agent_position)
{
  BLOG_TRACE_STARTED();
  const std::vector<wcm::Agent>& agents = _wizard_bucket.get_agents();
  const bool succeeded = not agents.empty() and agents.size() > agent_position;
  if (!succeeded)
    throw
      wc::Exception(
          as_function(__PRETTY_FUNCTION__)
          + ": " + as_error("vector 'agents' is empty"));
  const wcm::Agent& va = agents[agent_position];
  const std::string& va_last_name = va.get_last_name();
  const std::string& va_id = va.get_agent_id();

  BLOG_TRACE(LogItem(
      undefined,
      wc::concat("va_last_name=", va_last_name, ", va_id=", va_id)));
  BLOG_TRACE_FINISHED();
  return va_id;
}

const std::set<std::string>
we::Workflow::get_vateam_guids() {
  BLOG_TRACE_STARTED();
  const auto relations = _wizard_bucket.get_branchgroup().get_relations();
  const bool succeeded = not relations.empty();
  if (!succeeded)
    throw
      wc::Exception(
          as_function(__PRETTY_FUNCTION__)
          + ": " + as_error("branchgorup relation vector is empty"));
  std::set<std::string> vateam_guids;
  for (const auto relation : relations) {
    const auto& vagroup_guid = relation.get_vagroup_id();
    vateam_guids.insert(vagroup_guid);

    BLOG_TRACE(LogItem(undefined, wc::concat("vateam got ok=", vagroup_guid)));
  }
  BLOG_TRACE_FINISHED();
  return vateam_guids;
}

const std::set<std::string>
we::Workflow::get_vacenter_guids() {
  BLOG_TRACE_STARTED();
  const auto& vacenter = _wizard_bucket.get_vacenter();
  const std::string vacenter_guid = vacenter.getGuid();
  std::set<std::string> vacenter_guids;
  vacenter_guids.insert(vacenter_guid);
  BLOG_TRACE_FINISHED();
  return vacenter_guids;
}

const std::string
we::Workflow::get_branch_guid(){
  BLOG_TRACE_STARTED();
  const auto& branch_guid = _wizard_bucket.get_branch().get_main_id();
  BLOG_TRACE_FINISHED();
  return branch_guid;
}

const std::string we::Workflow::get_branchgroup_delfosnar_guid( uint branchgroup_position ){
  BLOG_TRACE_STARTED();
  const std::vector<wcm::BranchGroup>& branchgroups = _wizard_bucket.get_branchgroups_delfosnar();
  const bool succeeded = not branchgroups.empty() and branchgroups.size() > branchgroup_position;
  if (!succeeded)
    throw
      wc::Exception(
          as_function(__PRETTY_FUNCTION__)
          + ": " + as_error("vector 'agents' is empty"));
  const wcm::BranchGroup& bg = branchgroups[branchgroup_position];
  const std::string& guid = bg.getGuid();

  BLOG_TRACE(LogItem(
      undefined,
      wc::concat("brangchgroup_guid=", guid)));
  BLOG_TRACE_FINISHED();
  return guid;
}
const std::string we::Workflow::get_branch_delfosnar_guid( uint branch_position ){
  BLOG_TRACE_STARTED();
  const std::vector<wcm::Branch>& branches = _wizard_bucket.get_branches_delfosnar();
  const bool succeeded = not branches.empty() and branches.size() > branch_position;
  if (!succeeded)
    throw
      wc::Exception(
          as_function(__PRETTY_FUNCTION__)
          + ": " + as_error("vector 'agents' is empty"));
  const wcm::Branch& b = branches[branch_position];
  const std::string& guid = b.getGuid();

  BLOG_TRACE(LogItem(
      undefined,
      wc::concat("branch_guid=", guid)));
  BLOG_TRACE_FINISHED();
  return guid;
}

// ------------------
// RequestTrace methods
// ------------------

void we::Workflow::init_trace(const delfos::core::lm_event_type event){
  _trace = wcm::RequestTrace(_workflow_name, wc::as_string(event), _workflow_id);
}

void we::Workflow::init_trace(const delfos::core::gari_event_type event){
  _trace = wcm::RequestTrace(_workflow_name, wc::as_string(event), _workflow_id);
}

void we::Workflow::end_trace(const bool& request_success, const WJson& json_response){
  _trace.set_response(request_success, json_response);

  EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
  const bool enable = configuration->get_bool(EnzoConfiguration::KEY_STR_REQUEST_TRACE_ENABLE);

  if( enable )
    _request_traces->get_object_nodes()->auto_insert_node(_trace.get_guid(), _trace);
}

void we::Workflow::init_trace_alive(const delfos::core::gari_event_type event){
  _trace_alive = wcm::RequestTrace(_workflow_name, wc::as_string(event), _workflow_id);
}

void we::Workflow::init_trace_alive(const delfos::core::lm_event_type event){
  _trace_alive = wcm::RequestTrace(_workflow_name, wc::as_string(event), _workflow_id);
}

void we::Workflow::end_trace_alive(const bool& request_success, const WJson& json_response){
  _trace_alive.set_response(request_success, json_response);

  EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
  const bool enable = configuration->get_bool(EnzoConfiguration::KEY_STR_REQUEST_TRACE_ENABLE);

  if( enable )
    _request_traces->get_object_nodes()->auto_insert_node(_trace_alive.get_guid(), _trace_alive);
}



// ------------------
// Alive methods
// ------------------

void we::Workflow::gari_create_seed(){

  wcm::Room seed;

  wcm::ChatFilePod* file_pod = new wcm::ChatFilePod(WORKFLOW_CHATFILE_POD);
  seed.test_add_pod( file_pod );

  wcm::CameraPod* camera_pod = new wcm::CameraPod(WORKFLOW_CAMERA_POD);
  camera_pod->set_camera_on(true);
  seed.test_add_pod(camera_pod);

  wcm::SharingPod* sharing_pod = new wcm::SharingPod(WORKFLOW_SHARING_POD);
  list<unsigned int> slide_selector = {0};
  sharing_pod->test_set_slide_selector(slide_selector);
  string file_tree = "{\"255e1ffd-ea00-4432-b358-3581e5af09ec\":{" \
                          "\"url\":\"//static.delfos.com/PrivateResources/255e1ffd-ea00-4432-b358-3581e5af09ec.jpg\","\
                          "\"ext\":\"jpg\","\
                          "\"name\":\"Porsche Pr\","\
                          "\"children\":{},"\
                          "\"tree_key\":[]"\
                          "}}";
  sharing_pod->test_set_file_tree(file_tree);
  seed.test_add_pod(sharing_pod);


  WJson json_response;
  _seed_guid = NewGuid();
  init_trace(wc::gari_event_type::create_room);
  bool request_succeeded =
      _gari_tester.request_create_seed(_seed_guid, seed, json_response);
  end_trace(request_succeeded, json_response);
  if (!request_succeeded) {
    BLOG_FATAL(
      LogItem( wc::gari_event_type::create_room, wc::concat("failed")));
    throw wc::Exception( as_function(__PRETTY_FUNCTION__) + ": " + as_error("not working"));
  }
  return;
}

const wcm::ChatFilePod we::Workflow::generate_chat_file_pod(unsigned int source_id){
  // Generate ChatMessage update pod
  wcm::ChatMessage chat_message;
  chat_message.set_type(0);
  chat_message.set_content("mycontent");
  chat_message.set_source(0);
  chat_message.set_source_id( source_id );
  wcm::ChatFilePod file_pod(WORKFLOW_CHATFILE_POD);
  file_pod.push_back_msg(chat_message);

  return file_pod;
}

bool we::Workflow::gari_update_room(const std::string& gari_session_guid){

  // Get a dummy chat file pod
  wcm::ChatFilePod file_pod = generate_chat_file_pod( _gari_user_guid );

  // Perform query
  WJson json_response;
  init_trace(wc::gari_event_type::update_room);
  bool request_succeeded =
      _gari_tester.request_update(gari_session_guid,
                                  _gari_user_guid,
                                  WORKFLOW_CHATFILE_POD,
                                  &file_pod,
                                  json_response);
  end_trace(request_succeeded, json_response);
  if (!request_succeeded) {
    BLOG_FATAL(
      LogItem( wc::gari_event_type::update_room, wc::concat("failed ", json_response.to_str())));
    return false;
  }
  return true;
}


void we::Workflow::alive_lm(const std::string& agent_guid,
                            const std::string& session_guid,
                            const std::string& lead_guid)
{
  WJson json_response;
  bool request_succeeded;
  init_trace_alive(wc::lm_event_type::desktop_alive);
  if( lead_guid.empty() ){
    request_succeeded = _lm_tester.request_alive(agent_guid, session_guid, json_response);
  }else{
    request_succeeded = _lm_tester.request_alive(agent_guid, session_guid, lead_guid, json_response);
  }
  end_trace_alive(request_succeeded, json_response);
  if (!request_succeeded) {
    BLOG_FATAL(
      LogItem( wc::lm_event_type::desktop_alive,
                wc::concat("failed for agent '", agent_guid, "'", json_response.to_str())));
    throw wc::Exception( as_function(__PRETTY_FUNCTION__) + ": " + as_error("not working"));
  }
}



void we::Workflow::alive_gari(const std::string& lead_guid){
  WJson json_response;
  bool request_succeeded = false;

  if( _gari_user_guid == 0 ){

    init_trace_alive(wc::gari_event_type::read_room);
    request_succeeded =
        _gari_tester.request_read_session(lead_guid, wc::room_user_type::user, json_response);
    end_trace_alive(request_succeeded, json_response);
    if (!request_succeeded) {
      BLOG_FATAL(
        LogItem( wc::gari_event_type::read_room, wc::concat("failed ", json_response.to_str())));
      throw wc::Exception( as_function(__PRETTY_FUNCTION__) + ": " + as_error("not working"));
    }
    // Store user_id
    if(json_response.HasMember(wcm::ROOM_KEY_STR_USER_ID) and
      json_response[wcm::ROOM_KEY_STR_USER_ID].IsUint() ){
      _gari_user_guid = json_response[wcm::ROOM_KEY_STR_USER_ID].GetUint();
    }

  }else{
    init_trace_alive(wc::gari_event_type::read_room);
    request_succeeded =
        _gari_tester.request_read_session_user(lead_guid, _gari_user_guid, json_response);
    end_trace_alive(request_succeeded, json_response);
    if (!request_succeeded) {
      BLOG_FATAL(
        LogItem( wc::gari_event_type::read_room, wc::concat("failed", json_response.to_str())));
      throw wc::Exception( as_function(__PRETTY_FUNCTION__) + ": " + as_error("not working"));
    }
  }
}


