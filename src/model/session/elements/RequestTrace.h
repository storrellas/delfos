/*
 * RequestStats.h
 *
 *  Created on: Feb 20, 2017
 *      Author: storrellas
 */

#pragma once

#include <chrono>

#include <wcore/utils/objects/WJson.h>
#include <wcore/utils/helper/Helper.h>

namespace delfos{
namespace core{
namespace model{


class RequestTrace : public WObject{
private:
  WJson _request;
  WJson _response;
  std::chrono::system_clock::time_point _request_timestamp;
  uint _duration;   // [ms]
  std::string _guid;
  std::string _command;
  uint _thread_id;
  std::string _workflow_name;
  bool _success;

public:

  constexpr static char const* KEY_STR_GUID = "guid";
  constexpr static char const* KEY_STR_WORKFLOW_NAME = "workflow_name";
  constexpr static char const* KEY_STR_SUCCESS = "success";
  constexpr static char const* KEY_STR_COMMAND = "command";
  constexpr static char const* KEY_STR_THREAD_ID = "thread_id";
  constexpr static char const* KEY_STR_REQUEST_JSON = "request";
  constexpr static char const* KEY_STR_REQUEST_TIMESTAMP = "request_timestamp";
  constexpr static char const* KEY_STR_RESPONSE_JSON = "response";
  constexpr static char const* KEY_STR_RESPONSE_TIMESTAMP = "response_timestamp";
  constexpr static char const* KEY_STR_DURATION = "duration";

  RequestTrace();
  RequestTrace(const std::string& workflow_name,
               const std::string& command, const uint& thread_id);
  RequestTrace(const RequestTrace& other);
  RequestTrace& operator=(RequestTrace other);
  bool operator==(RequestTrace other) const;
  virtual ~RequestTrace();
  virtual void swap(WObject& other);
  virtual void clear();

  WObject* create() const
  {
    return new RequestTrace();
  }
  WObject* clone() const
  {
    return new RequestTrace(*this);
  }
  WObject* copy_to(WObject* obj) const
  {
    if (obj && obj != this)
    {
      RequestTrace* o = dynamic_cast<RequestTrace*>(obj);
      if (o)
      {
        *o = *this;
      }
    }
    return obj;
  }


  /// Getters
  const std::string& get_guid() const { return _guid; }
  const std::string& get_workflow_name() const { return _workflow_name; }
  const bool& get_success() const { return _success; }
  const std::string& get_command() const { return _command; }
  const uint& get_thread_id() const { return _thread_id; }
  const WJson& get_request() const { return _request; }
  const WJson& get_response() const { return _response; }
  const std::chrono::system_clock::time_point get_request_timepoint() const {
    return _request_timestamp;
  }
  const std::string get_request() { return _request.to_str(); }
  const std::string get_response() { return _response.to_str(); }
  const std::string get_request_timestamp() {
    return delfos::core::as_db_string(_request_timestamp);
  }
  uint get_duration() const { return _duration; }

  /// Setters
  void set_guid(const std::string& guid) { _guid = guid; };
  void set_workflow_name(const std::string& guid) { _workflow_name = guid; };
  void set_command(const std::string& value) { _command = value; }
  void set_thread_id(const uint& value) { _thread_id = value; }
  void set_request(const WJson& request) {
    _request = request;
  }
  void set_response(const bool& success, const WJson& response) {
    set_duration();
    _success = success;
    _response = response;
  }
  void set_request_timestamp(const std::chrono::system_clock::time_point& request_timestamp = std::chrono::system_clock::now()) {
    _request_timestamp = request_timestamp;
  }
  uint set_duration();

  WJson& append_to_json_obj(unsigned int selector, WJson& json) const;

};

}
}
}


