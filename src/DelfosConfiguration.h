/*
 * RMConfigController.h
 *
 *  Created on: Dec 21, 2016
 *      Author: storrellas
 */

#pragma once

#include <wcore/utils/containers/model_structures/Configuration.h>


class EnzoConfiguration :
    public delfos::core::Configuration
{
private:
  EnzoConfiguration();
  virtual ~EnzoConfiguration();

private:
  static EnzoConfiguration* _instance;
public:

  /**
   * Returns an instance
   * @return
   */
  static EnzoConfiguration* get_instance(){
    if( _instance == nullptr )
      _instance = new EnzoConfiguration();
    return _instance;
  }

  // Config keys
  static constexpr const char* CONFIG_FILE_PATH = "/etc/delfos/delfosEnzo.conf";

  static constexpr const char* KEY_STR_APP_LEVEL_LOG = "app_level_log";
  static constexpr const char* KEY_STR_APP_LEVEL_SYSLOG = "app_level_syslog";

  // DAI Configuration
  static constexpr const char* KEY_STR_DAI_IP = "dai_ip";
  static constexpr const char* KEY_STR_DAI_PORT = "dai_port";
  static constexpr const char* KEY_STR_DAI_DB_ENGINE = "dai_db_engine";
  static constexpr const char* KEY_STR_DAI_DB_CONFIGURATION = "dai_db_configuration";
  static constexpr const char* KEY_STR_DAI_USERNAME = "dai_username";
  static constexpr const char* KEY_STR_DAI_PASSWORD = "dai_password";

  // Cache
  static constexpr const char* KEY_STR_CACHE_SYSTEM = "cache_system";
  static constexpr const char* KEY_STR_CACHE_REDIS_IP = "cache_system_ip";
  static constexpr const char* KEY_STR_CACHE_REDIS_PORT = "cache_system_port";

  // Core Targeted
  static constexpr const char* KEY_STR_LM_IP = "lm_ip";
  static constexpr const char* KEY_STR_LM_PORT_HTTP = "lm_port_http";
  static constexpr const char* KEY_STR_GARI_IP = "gari_ip";
  static constexpr const char* KEY_STR_GARI_PORT_HTTP = "gari_port_http";
  static constexpr const char* KEY_STR_PBX_IP = "pbx_ip";
  static constexpr const char* KEY_STR_PBX_PORT = "pbx_port";

  // Twilio Mockup
  static constexpr const char* KEY_STR_TWILIO_PBX_IP   = "twilio_pbx_ip";
  static constexpr const char* KEY_STR_TWILIO_PBX_PORT = "twilio_pbx_port";

  // Workflow
  static constexpr const char* KEY_STR_DELTA_MIN      = "delta_min";
  static constexpr const char* KEY_STR_DELTA_MAX      = "delta_max";
  static constexpr const char* KEY_STR_DELTA_PERIODIC = "delta_periodic";

  static constexpr const char* KEY_STR_AGENT_WORKFLOW_AGENT_NUMBER   = "agent_workflow_agent_number";
  static constexpr const char* KEY_STR_CMN_WORKFLOW_AGENT_NUMBER   = "cmn_workflow_agent_number";
  static constexpr const char* KEY_STR_IV_WORKFLOW_AGENT_NUMBER   = "iv_workflow_agent_number";
  static constexpr const char* KEY_STR_INBOUND_WORKFLOW_AGENT_NUMBER   = "inbound_workflow_agent_number";
  static constexpr const char* KEY_STR_SCHEDULED_WORKFLOW_AGENT_NUMBER   = "scheduled_workflow_agent_number";
  static constexpr const char* KEY_STR_EXPIRED_WORKFLOW_AGENT_NUMBER   = "expired_workflow_agent_number";
  static constexpr const char* KEY_STR_WHISBINAR_WORKFLOW_AGENT_NUMBER   = "delfosnar_workflow_agent_number";

  // Request Trace
  static constexpr const char* KEY_STR_REQUEST_TRACE_ENABLE   = "request_trace_enable";
  static constexpr const char* KEY_STR_REQUEST_TRACE_DUMP_PERIOD   = "request_trace_dump_period";
  static constexpr const char* KEY_STR_REQUEST_TRACE_DUMP_AMOUNT   = "request_trace_dump_amount";

  static constexpr const char* KEY_STR_REQUEST_TRACE_DAI_IP = "request_trace_dai_ip";
  static constexpr const char* KEY_STR_REQUEST_TRACE_DAI_PORT = "request_trace_dai_port";
  static constexpr const char* KEY_STR_REQUEST_TRACE_DAI_DB_ENGINE = "request_trace_dai_db_engine";
  static constexpr const char* KEY_STR_REQUEST_TRACE_DAI_DB = "request_trace_dai_db";
  static constexpr const char* KEY_STR_REQUEST_TRACE_DAI_USERNAME = "request_trace_dai_username";
  static constexpr const char* KEY_STR_REQUEST_TRACE_DAI_PASSWORD = "request_trace_dai_password";

};



