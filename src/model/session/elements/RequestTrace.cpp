/*
 * RequestStats.cpp
 *
 *  Created on: Feb 20, 2017
 *      Author: storrellas
 */

#include "RequestTrace.h"


namespace wc = delfos::core;
namespace wcm = delfos::core::model;

using namespace std::chrono;

wcm::RequestTrace::RequestTrace() :
  _request(),
  _response(),
  _request_timestamp(std::chrono::system_clock::now()),
  _duration(0),
  _guid(NewGuid()),
  _command(),
  _thread_id(0),
  _workflow_name(),
  _success(false)
{
}

wcm::RequestTrace::RequestTrace(const std::string& workflow_name,
                                const std::string& command, const uint& thread_id) :
  _request(),
  _response(),
  _request_timestamp(std::chrono::system_clock::now()),
  _duration(0),
  _guid(NewGuid()),
  _command(command),
  _thread_id(thread_id),
  _workflow_name(workflow_name),
  _success(false)
{
}


wcm::RequestTrace::RequestTrace(const wcm::RequestTrace& other) :
  _request(other._request),
  _response(other._response),
  _request_timestamp(other._request_timestamp),
  _duration(other._duration),
  _guid(other._guid),
  _command(other._command),
  _thread_id(other._thread_id),
  _workflow_name(other._workflow_name),
  _success(other._success)
{
}

wcm::RequestTrace& wcm::RequestTrace::operator =(wcm::RequestTrace other)
{
  this->swap(other);
  return *this;
}

bool wcm::RequestTrace::operator ==(wcm::RequestTrace other) const
{
  return _request == other._request and
         _response == other._response and
         _request_timestamp == other._request_timestamp and
         _duration == other._duration and
         _guid == other._guid and
         _command == other._command and
         _thread_id == other._thread_id and
         _workflow_name ==_workflow_name and
         _success == _success;
}

void wcm::RequestTrace::swap(WObject& other)
{
  RequestTrace& o = static_cast<RequestTrace&>(other);
  std::swap(_request, o._request);
  std::swap(_response, o._response);
  std::swap(_request_timestamp, o._request_timestamp);
  std::swap(_duration, o._duration);
  std::swap(_guid, o._guid);
  std::swap(_command, o._command);
  std::swap(_thread_id, o._thread_id);
  std::swap(_workflow_name, o._workflow_name);
  std::swap(_success, o._success);
}

void wcm::RequestTrace::clear()
{
  _request.Clear();
  _response.Clear(),
  _request_timestamp = std::chrono::system_clock::now();
  _duration = 0;
  _guid.clear();
  _command.clear();
  _thread_id = 0;
  _workflow_name.clear();
  _success = false;
}

wcm::RequestTrace::~RequestTrace(){
}

WJson& wcm::RequestTrace::append_to_json_obj(unsigned int, WJson& json) const{
  json.append_pair_to_json(KEY_STR_GUID, _guid);
  json.append_pair_to_json(KEY_STR_WORKFLOW_NAME, _workflow_name);
  json.append_pair_to_json(KEY_STR_SUCCESS, _success);
  json.append_pair_to_json(KEY_STR_COMMAND, _command);
  json.append_pair_to_json(KEY_STR_THREAD_ID, _thread_id);
  json.append_pair_to_json(KEY_STR_REQUEST_JSON, _request.to_str());
  json.append_pair_to_json(KEY_STR_REQUEST_TIMESTAMP, delfos::core::as_db_string(_request_timestamp) );
  json.append_pair_to_json(KEY_STR_RESPONSE_JSON, _response.to_str());
  json.append_pair_to_json(KEY_STR_DURATION, _duration);
  return json;
}

uint wcm::RequestTrace::set_duration() {
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  _duration = duration_cast<milliseconds>(now - _request_timestamp).count();
  return _duration;
}
