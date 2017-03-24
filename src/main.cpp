// --

#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include <wcore/logging/Logger.h>
#include <wcore/utils/unit_test/mockups/TwilioMockup.h>
#include <wcore/enums/enums.h>
#include <wcore/constants/lm_constants.h>

#include <src/utils/Barrier.h>
#include <src/utils/cli-parser.h>
#include <src/utils/spy.h>

#include <src/WizardBucketRAII.h>
#include <src/EnzoConfiguration.h>
#include <src/RequestTraceThreadController.h>
#include <src/workflows/AgentWorkflow.h>
#include <src/workflows/CMNWorkflow.h>
#include <src/workflows/IVWorkflow.h>
#include <src/workflows/InboundWorkflow.h>
#include <src/workflows/ScheduledWorkflow.h>
#include <src/workflows/ExpiredWorkflow.h>
#include <src/workflows/delfosnarWorkflow.h>

namespace wc = delfos::core;
namespace wcc = delfos::core::constants;
namespace wcm = delfos::core::model;
namespace wcct = delfos::core::constants::tags;
namespace we = delfos::enzo;

namespace {

  const wc::void_event_type undefined = wc::void_event_type::undefined;
  constexpr wc::Alert error = wc::Alert::error;
  constexpr wc::Alert warning = wc::Alert::warning;


  class TwilioEvent : public TwilioEventHandler{
  private:
    const std::list<we::Workflow*>& _workflow_list;
    const std::atomic<bool>& _process_can_continue;
  public:
    TwilioEvent(const std::list<we::Workflow*>& workflow_list, std::atomic<bool>& process_can_continue):
      _workflow_list(workflow_list),
      _process_can_continue(process_can_continue)
    {}
    virtual ~TwilioEvent(){}

    bool url_contains(std::string url_candidate, std::string pattern){
      return url_candidate.find(pattern) != std::string::npos;
    }

    bool twilio_event(const WHttp http_request){

      // NOTE: Avoid to call workflow being deleted
      if( not _process_can_continue.load() )
        return true;

      // Signal all workflows of the the twilio_event
      for( auto item : _workflow_list)
        item->twilio_event(http_request);
      return true;
    }

    bool twilio_request(const WHttp http_request, const WHttp http_response){
      // NOTE: Avoid to call workflow being deleted
      if( not _process_can_continue.load() )
        return true;

      // Signal all workflows of the the twilio_event
      for( auto item : _workflow_list)
        item->twilio_request(http_request, http_response);
      return true;
    }


  };

  TwilioMockup* twilio_mockup = nullptr;
  TwilioEvent* twilio_event = nullptr;
  we::Barrier barrier;
  std::atomic<bool> process_can_continue(true);
  // List to store workflows
  std::list<we::Workflow*> workflow_list;
  // General logger
  delfosErrorHandler weh;
  // Nodes to Store RequestTraces
  RequestTraceNodes* request_traces = nullptr;

  const int ENZO_PBX_CHECK_RETRY = 10;
  const int ENZO_PBX_CHECK_PERIOD_SECONDS = 3;

  void terminate_process(int) {
    BLOG_TRACE_STARTED();
    process_can_continue.store(false);
    BLOG_INFO(LogItem(undefined, "term/int signal received, ending threads..."));
    BLOG_TRACE_FINISHED();
  }
  void
  set_term_and_int_signals() {
    WSignal signal;
    signal.add_signal_handler(SIGTERM, terminate_process);
    signal.add_signal_handler(SIGINT, terminate_process);
    signal.add_signal_handler(SIGUSR1, terminate_process);
    signal.apply_handlers();
  }

  std::string exec(const char* cmd) {
      std::array<char, 128> buffer;
      std::string result;
      std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
      if (!pipe) throw std::runtime_error("popen() failed!");
      while (!feof(pipe.get())) {
          if (fgets(buffer.data(), 128, pipe.get()) != NULL)
              result += buffer.data();
      }
      return result;
  }

  bool request_ping_to_pbx(){
    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    const string pbx_ip = configuration->get_string(EnzoConfiguration::KEY_STR_PBX_IP);
    const string pbx_port = configuration->get_string(EnzoConfiguration::KEY_STR_PBX_PORT);


    const string path = "/ping/";
    const string body = "";
    WHttp request(W_HTTP_GET, W_HTTP_PROTOCOL_1_1, wcc::localhost_ip, path, &body);
    request.push_header(W_HTTP_HEADER_KEY_CONTENT_TYPE,
                        W_HTTP_HEADER_VALUE_CONTENT_TYPE_APP_JSON);

    // Perform request
    WErrorStack es;
    WHttp http_response;
    delfosSocketsManager wsm(&weh, nullptr);
    wsm.do_send_and_receive_http(pbx_ip, pbx_port, request, false, false, http_response, es);
    if( !es.is_empty() ){
      BLOG_ERROR(LogItem(undefined, es));
      return false;
    }
    return true;
  }

  bool check_pbx_running(){
//    // Check whether PBX is running - locally
//    const string command = "ps -aux | grep bin/delfosPBXServer | grep -v grep";
//    const std::string result = exec( command.c_str() );
//    const bool pbx_running = not result.empty();


    for(int i = 0; i < ENZO_PBX_CHECK_RETRY; ++i){

      // Check whether PBX is running
      const bool pbx_running = request_ping_to_pbx();
      if( pbx_running ){
        BLOG_INFO( LogItem(undefined, "PBX is running") );
        return true;
      }else{
        BLOG_INFO( LogItem(undefined, "PBX is NOT running") );
      }

      // Wait some time for next checking
      std::this_thread::sleep_for(std::chrono::seconds(ENZO_PBX_CHECK_PERIOD_SECONDS));
    }

    return false;
  }

} // namespace


namespace Workflow{


  void launch_agent(const WizardBucketRAII& wizard_bucket_raii,
                            const uint start_agent_index){

    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    uint n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_AGENT_WORKFLOW_AGENT_NUMBER);
    for (size_t i = 0; i < n_agents; ++i) {
      we::Workflow* workflow =
          new we::AgentWorkflow(
              process_can_continue,
              barrier,
              wizard_bucket_raii.get(),
              &weh,
              request_traces,
              start_agent_index + i);
      workflow->start();
      workflow_list.push_back(workflow);
    }

    BLOG_INFO(
        LogItem(undefined, wc::concat("all ", we::AgentWorkflow::get_workflow_name() ," workflows[", n_agents, "] started OK!")));

  }

  void launch_cmn(const WizardBucketRAII& wizard_bucket_raii,
                            const uint start_agent_index){

    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    uint n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_CMN_WORKFLOW_AGENT_NUMBER);
    for (size_t i = 0; i < n_agents; ++i) {
      we::Workflow* workflow =
          new we::CMNWorkflow(
              process_can_continue,
              barrier,
              wizard_bucket_raii.get(),
              &weh,
              request_traces,
              start_agent_index + i);
      workflow->start();
      workflow_list.push_back(workflow);
    }

    BLOG_INFO(
        LogItem(undefined, wc::concat("all ", we::CMNWorkflow::get_workflow_name() ," workflows[", n_agents, "] started OK!")));
  }

  void launch_iv(const WizardBucketRAII& wizard_bucket_raii,
                            const uint start_agent_index){

    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    uint n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_IV_WORKFLOW_AGENT_NUMBER);
    for (size_t i = 0; i < n_agents; ++i) {
      we::Workflow* workflow =
          new we::IVWorkflow(
              process_can_continue,
              barrier,
              wizard_bucket_raii.get(),
              &weh,
              request_traces,
              start_agent_index + i,
              twilio_mockup);
      workflow->start();
      workflow_list.push_back(workflow);
    }

    BLOG_INFO(
        LogItem(undefined, wc::concat("all ", we::IVWorkflow::get_workflow_name() ," workflows[", n_agents, "] started OK!")));
  }

  void launch_inbound(const WizardBucketRAII& wizard_bucket_raii,
                            const uint start_agent_index){

    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    uint n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_INBOUND_WORKFLOW_AGENT_NUMBER);
    for (size_t i = 0; i < n_agents; ++i) {
      we::Workflow* workflow =
          new we::InboundWorkflow(
              process_can_continue,
              barrier,
              wizard_bucket_raii.get(),
              &weh,
              request_traces,
              start_agent_index + i,
              twilio_mockup);
      workflow->start();
      workflow_list.push_back(workflow);
    }

    BLOG_INFO(
        LogItem(undefined, wc::concat("all ", we::InboundWorkflow::get_workflow_name() ," workflows[", n_agents, "] started OK!")));
  }

  void launch_scheduled(const WizardBucketRAII& wizard_bucket_raii,
                            const uint start_agent_index){

    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    uint n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_SCHEDULED_WORKFLOW_AGENT_NUMBER);
    for (size_t i = 0; i < n_agents; ++i) {
      we::Workflow* workflow =
          new we::ScheduledWorkflow(
              process_can_continue,
              barrier,
              wizard_bucket_raii.get(),
              &weh,
              request_traces,
              start_agent_index + i);
      workflow->start();
      workflow_list.push_back(workflow);
    }

    BLOG_INFO(
        LogItem(undefined, wc::concat("all ", we::ScheduledWorkflow::get_workflow_name() ," workflows[", n_agents, "] started OK!")));
  }

  void launch_expired(const WizardBucketRAII& wizard_bucket_raii,
                            const uint start_agent_index){

    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    uint n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_EXPIRED_WORKFLOW_AGENT_NUMBER);
    for (size_t i = 0; i < n_agents; ++i) {
      we::Workflow* workflow =
          new we::ExpiredWorkflow(
              process_can_continue,
              barrier,
              wizard_bucket_raii.get(),
              &weh,
              request_traces,
              start_agent_index + i);
      workflow->start();
      workflow_list.push_back(workflow);
    }

    BLOG_INFO(
        LogItem(undefined, wc::concat("all ", we::ExpiredWorkflow::get_workflow_name() ," workflows[", n_agents, "] started OK!")));
  }

  void launch_delfosnar(const WizardBucketRAII& wizard_bucket_raii,
                            const uint start_agent_index){

    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    uint n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_WHISBINAR_WORKFLOW_AGENT_NUMBER);
    for (size_t i = 0; i < n_agents; ++i) {
      we::Workflow* workflow =
          new we::delfosnarWorkflow(
              process_can_continue,
              barrier,
              wizard_bucket_raii.get(),
              &weh,
              request_traces,
              start_agent_index + i);
      workflow->start();
      workflow_list.push_back(workflow);
    }

    BLOG_INFO(
        LogItem(undefined, wc::concat("all ", we::delfosnarWorkflow::get_workflow_name() ," workflows[", n_agents, "] started OK!")));
  }

}

int
main(const int argc, char* argv[]) {
  bool succeeded = cli::parse_command_line(argc, argv);
  if (!succeeded)
    return EXIT_FAILURE;

  std::cout
    << spy::RunInfo(cli::argv_0(), cli::program_name()) << '\n'
    << cli::parsed_command_line << std::endl;

  /// Generate logger
  wc::Logger
    logger(
      cli::fake_module,
      cli::fake_instance,
      wc::severity_level::trace,
      wc::severity_level::trace);
  BLOG_TRACE(LogItem(undefined, "logger created"));

  try {
    weh.StartEH("enzo", Default, false);

    /// Load configuration
    WErrorStack es;
    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    if(!configuration->load(EnzoConfiguration::CONFIG_FILE_PATH, es)){
      cout << "Failed to load configuration " << endl;
      return EXIT_FAILURE;
    }
    BLOG_INFO( LogItem(undefined, "Loaded configuration") );
    cout << configuration->to_str() << endl;

    /// Initilialise TwilioFake
    const string pbx_twilio_ip = configuration->get_string(EnzoConfiguration::KEY_STR_TWILIO_PBX_IP);
    const string pbx_twilio_port = configuration->get_string(EnzoConfiguration::KEY_STR_TWILIO_PBX_PORT);
    twilio_event = new TwilioEvent(workflow_list, process_can_continue);
    twilio_mockup = new TwilioMockup(&weh);
    twilio_mockup->set_connection_parameters(pbx_twilio_ip, pbx_twilio_port);
    twilio_mockup->set_event_handler(twilio_event);
    twilio_mockup->init();


    logger.set_log_min_severity_syslog(wc::severity_level::debug);
    logger.set_log_min_severity_console(wc::severity_level::debug);
    if( not check_pbx_running() ){
      BLOG_ERROR( LogItem(undefined, "delfosPBXServer was not found. Aborting Enzo") );
      return EXIT_FAILURE;
    }
    BLOG_INFO( LogItem(undefined, "delfosPBXServer running. Proceeding to start Enzo ...") );

    // Configure signals
    set_term_and_int_signals();

    // Configure logger levels
    const uint app_level_log = configuration->get_uint(EnzoConfiguration::KEY_STR_APP_LEVEL_LOG);
    const uint app_level_syslog = configuration->get_uint(EnzoConfiguration::KEY_STR_APP_LEVEL_SYSLOG);
    logger.set_log_min_severity_syslog(wc::as<wc::severity_level>(app_level_log));
    logger.set_log_min_severity_console(wc::as<wc::severity_level>(app_level_syslog));

    /// create DB model
    wcore::SmartModel::DAIConf dai_conf;
    dai_conf.dai_ip = configuration->get_string(EnzoConfiguration::KEY_STR_DAI_IP);
    dai_conf.dai_port = configuration->get_string(EnzoConfiguration::KEY_STR_DAI_PORT);
    dai_conf.dai_db_conn  = configuration->get_string(EnzoConfiguration::KEY_STR_DAI_DB_CONFIGURATION);
    const string dai_db_engine_str = configuration->get_string(EnzoConfiguration::KEY_STR_DAI_DB_ENGINE);
    dai_conf.dai_db_engine = wc::as_db_engine(dai_db_engine_str);
    if(dai_conf.dai_db_engine == wc::db_engine::undefined)
      dai_conf.dai_db_engine = wc::db_engine::sqlserver;
    dai_conf.dai_username = configuration->get_string(EnzoConfiguration::KEY_STR_DAI_USERNAME);
    dai_conf.dai_password = configuration->get_string(EnzoConfiguration::KEY_STR_DAI_PASSWORD);
    wc::SQLSyntaxSingleton& sql_syntax = wc::SQLSyntaxSingleton::getInstance();
    sql_syntax.set_sql_syntax(dai_conf.dai_db_engine);

    const uint agent_n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_AGENT_WORKFLOW_AGENT_NUMBER);
    const uint cmn_n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_CMN_WORKFLOW_AGENT_NUMBER);
    const uint iv_n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_IV_WORKFLOW_AGENT_NUMBER);
    const uint inbound_n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_INBOUND_WORKFLOW_AGENT_NUMBER);
    const uint scheduled_n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_SCHEDULED_WORKFLOW_AGENT_NUMBER);
    const uint expired_n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_EXPIRED_WORKFLOW_AGENT_NUMBER);
    const uint delfosnar_n_agents = configuration->get_uint(EnzoConfiguration::KEY_STR_WHISBINAR_WORKFLOW_AGENT_NUMBER);
    const uint total_agents = agent_n_agents + cmn_n_agents + iv_n_agents +
                                inbound_n_agents + scheduled_n_agents + expired_n_agents + delfosnar_n_agents;

    // this ctor may throw an exception DB-related
    const WizardBucketRAII wizard_bucket_raii(dai_conf, total_agents);
    BLOG_INFO(LogItem(undefined, "Wizard Bucket Created!"));

    /// Initialise RequestTraceNodes
    const string cache_redis_ip = configuration->get_string(EnzoConfiguration::KEY_STR_CACHE_REDIS_IP);
    const string cache_redis_port = configuration->get_string(EnzoConfiguration::KEY_STR_CACHE_REDIS_PORT);
    const string cache_system_str = configuration->get_string(EnzoConfiguration::KEY_STR_CACHE_SYSTEM);
    wcore::SmartModel::RedisConf redis_conf(cache_redis_ip, cache_redis_port);
    wc::cache_system cache_system = wc::as_cache_system(cache_system_str);
    request_traces = new RequestTraceNodes(&weh);
    request_traces->init(&weh, cache_system,
                          wcc::NODES_MAX_RETRIES, wcc::NODES_INTERVAL_TO_NEXT_RETRY,
                          dai_conf, redis_conf);

    /// Initialise RequestTrace Thread Controller to store in DB
    RequestTraceThreadController th(&weh, request_traces, cli::clear_request_trace, workflow_list);
    const uint dump_period = configuration->get_uint(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DUMP_PERIOD);
    th.set_sleep_period(dump_period);
    string error_msg;
    const bool enable = configuration->get_bool(EnzoConfiguration::KEY_STR_REQUEST_TRACE_ENABLE);
    if( enable)
      th.start_thread(&error_msg);
    if(!error_msg.empty()){
      BLOG_FATAL(
          LogItem(undefined, "Error starting RequestTraceThreadController"));
      return EXIT_FAILURE;
    }

    /// launch all workflows
    barrier.set(total_agents);
    Workflow::launch_agent(wizard_bucket_raii, 0);
    Workflow::launch_cmn(wizard_bucket_raii, agent_n_agents);
    Workflow::launch_iv(wizard_bucket_raii, agent_n_agents + cmn_n_agents);
    Workflow::launch_inbound(wizard_bucket_raii, agent_n_agents + cmn_n_agents + iv_n_agents);
    Workflow::launch_scheduled(wizard_bucket_raii, agent_n_agents + cmn_n_agents +
                                                    iv_n_agents + inbound_n_agents);
    Workflow::launch_expired(wizard_bucket_raii, agent_n_agents + cmn_n_agents +
                                                    iv_n_agents + inbound_n_agents + scheduled_n_agents);
    Workflow::launch_delfosnar(wizard_bucket_raii, agent_n_agents + cmn_n_agents +
                                                    iv_n_agents + inbound_n_agents + scheduled_n_agents + expired_n_agents);


    BLOG_INFO(LogItem(undefined, "waiting for barrier..."));
    barrier.wait();
    BLOG_INFO(
        LogItem(undefined, wc::concat("all workflows [", total_agents,"] started OK, let's stress that core!")));

    /// Join their threads
    for (auto item: workflow_list){
      item->terminate();
    }

    BLOG_INFO(LogItem(undefined, "all workflows joined OK, exiting..."));

    /// Delete items
    for (auto item: workflow_list){
      delete item;
    }
    workflow_list.clear();

    // Stop thread before proceeding
    th.do_stuff();  // Dump last items if any
    th.stop_thread();
    twilio_mockup->Stop();
    delete twilio_mockup;
    delete twilio_event;
    delete request_traces;

  } catch (const wc::Exception& e) {
    BLOG_FATAL(
      LogItem(undefined, wc::concat("wc::Exception caught: ", e.what())));
    return EXIT_FAILURE;
  } catch (const std::exception& e) {
    BLOG_FATAL(
      LogItem(undefined, wc::concat("std::exception caught: ", e.what())));
    return EXIT_FAILURE;
  } catch (...) {
    BLOG_FATAL(LogItem(undefined, "Exception '...' caught: "));
    return EXIT_FAILURE;
  }

  BLOG_TRACE_FINISHED();
  return EXIT_SUCCESS;
}

// -- eof
