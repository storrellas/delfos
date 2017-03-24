/*
 * LmLeadThreadController.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: mjlopez
 */

#include <wcore/logging/Logger.h>

#include "RequestTraceThreadController.h"


namespace wc = delfos::core;
namespace wcm = delfos::core::model;
namespace we = delfos::enzo;

namespace {

const wc::lm_event_type undefined = wc::lm_event_type::undefined;

} // namespace



RequestTraceThreadController::RequestTraceThreadController(delfosErrorHandler *weh,
                                                          RequestTraceNodes* request_traces,
                                                          const bool clear_request_trace,
                                                          std::list<we::Workflow*>& workflow_list) :
		delfos::core::ThreadController("session_tc", 30, weh),
		_request_traces(request_traces),
		_dai_helper(wc::db_engine::mysql, weh),
		_workflow_list(workflow_list)
{
	wcore::LogConfig log_config;
	log_config.set_elastic_enabled(true);
	log_config.set_syslog_enabled(false);
	log_config.set_terminal_enabled(false);
	set_log_config(log_config);

	// Capture EnzoConfiguration
  EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
  wcore::SmartModel::DAIConf dai_conf;
  dai_conf.dai_ip = configuration->get_string(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DAI_IP);
  dai_conf.dai_port = configuration->get_string(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DAI_PORT);
  dai_conf.dai_db_conn  = configuration->get_string(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DAI_DB);
  const string dai_db_engine_str = configuration->get_string(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DAI_DB_ENGINE);
  dai_conf.dai_db_engine = wc::as_db_engine(dai_db_engine_str);
  if(dai_conf.dai_db_engine == wc::db_engine::undefined)
    dai_conf.dai_db_engine = wc::db_engine::sqlserver;
  dai_conf.dai_username = configuration->get_string(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DAI_USERNAME);
  dai_conf.dai_password = configuration->get_string(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DAI_PASSWORD);

  WErrorStack es;
  if( !_dai_helper.init(dai_conf.dai_ip, dai_conf.dai_port, dai_conf.dai_username, dai_conf.dai_password, es) ){
    BLOG_FATAL( LogItem(undefined, "Error initialising DAIHelper"));
  }

  _max_request_trace_amount = configuration->get_uint(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DUMP_AMOUNT);

  /// Remove RequestTrace if enabled
  if( clear_request_trace ){
    WJson result_rows_json;
    string query ="DELETE FROM RequestTrace";
    const string db = configuration->get_string(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DAI_DB);
    bool res = _dai_helper.perform_query(query, db, wc::DAIHelper::DAI_UNKNOWN_RESULT_NUMBER, result_rows_json, es);
    if( !res ){
      BLOG_FATAL(
          LogItem(undefined, wc::concat("Failed to remove RequestTrace")));
    }
  }

}

RequestTraceThreadController::~RequestTraceThreadController() {
}

void RequestTraceThreadController::do_stuff()
{
  // Calculate number of workflows running
  uint n_workflows = 0;
  for(auto item : _workflow_list)
    if(item->get_workflow_running())
      n_workflows++;


  // Check traces are present
  uint queue_length = _request_traces->get_object_nodes()->auto_size();
  if( queue_length == 0){
    BLOG_INFO(
        LogItem(undefined, wc::concat("Inserting [", queue_length, "] request trace to DB.", " Workflows running [", n_workflows, "]")));
    return;
  }


  // Empty queue
  while( queue_length > 0 ){

    std::list<string> index_list;
    _request_traces->get_object_nodes()->auto_get_main_indexes_list_copy(index_list);

    // Limit the number of elements to be inserted in this request
    if( queue_length > _max_request_trace_amount)
        index_list.resize(_max_request_trace_amount);
    BLOG_INFO(
        LogItem(undefined, wc::concat("Inserting [", index_list.size(), "] request trace to DB.",
                                      " Workflows running [", n_workflows, "]")));


    string query = "INSERT INTO RequestTrace ";
    query += "(";
    query += "Guid, ";
    query += "WorkflowName, ";
    query += "Success, ";
    query += "Command, ";
    query += "WorkflowId, ";
    query += "RequestTimestamp, ";
    query += "Request, ";
    query += "Response, ";
    query += "Duration ";
    query += ")";
    query += "VALUES ";
    for( auto item : index_list ){
      wcm::RequestTrace trace;
      _request_traces->get_object_nodes()->auto_get_content_copy_by_main_index(item, trace);
      query += "('" + trace.get_guid() + "', ";
      query += "'" + trace.get_workflow_name() + "', ";
      query += " " + std::to_string(trace.get_success()) + ", ";
      query += "'" + trace.get_command() + "', ";
      query += " " + std::to_string(trace.get_thread_id()) + ", ";
      query += "'" +  trace.get_request_timestamp() + "', ";
      query += "'', ";
      // ---
      query += "'" +  trace.get_response() + "', ";
      //query += "'', ";
      // ---
      query += " " +  std::to_string(trace.get_duration()) + " ";
      query += "),";
      _request_traces->get_object_nodes()->auto_delete_node_by_main_index(item);
    }
    query.erase(query.find_last_of(","));
//    cout << "+++++++++++++++++++++++" << endl;
//    cout << "Queue after " << _request_traces->get_object_nodes()->auto_size() << endl;
//    cout << "+++++++++++++++++++++++" << endl;

    // Dump into DB
    EnzoConfiguration* configuration = EnzoConfiguration::get_instance();
    const string db = configuration->get_string(EnzoConfiguration::KEY_STR_REQUEST_TRACE_DAI_DB);
    WJson result_rows_json;
    WErrorStack es;
    _dai_helper.perform_query(query, db, wc::DAIHelper::DAI_UNKNOWN_RESULT_NUMBER, result_rows_json, es);
    if( !es.is_empty() ){
      BLOG_FATAL(
          LogItem(undefined, wc::concat("ERROR INSERTING REQUEST TRACE")));
      return;
    }

    queue_length = _request_traces->get_object_nodes()->auto_size();

  }


}



