/*
 * CoreModuleNodes.h
 *
 *  Created on: Oct 17, 2016
 *      Author: storrellas
 */

#pragma once

// Model includes
#include <src/model/session/elements/RequestTrace.h>
#include <wcore/utils/containers/FactoryNodes.h>


class RequestTraceNodes {
private:
  delfosErrorHandler* _err_handler;
  delfos::core::InterfaceNodes* _object_nodes;

public:
  RequestTraceNodes(delfosErrorHandler*);
  virtual ~RequestTraceNodes();

  void init(delfosErrorHandler*, const delfos::core::cache_system& cache_system,
            size_t max_retries, time_t retry_interval,
            const wcore::SmartModel::DAIConf& dai_conf,
            const wcore::SmartModel::RedisConf& redis_conf);

  /**
   * Getter
   */
  delfos::core::InterfaceNodes* get_object_nodes() const { return _object_nodes; }

};



