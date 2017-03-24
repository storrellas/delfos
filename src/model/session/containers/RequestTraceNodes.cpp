/*
 * BranchNodes.cpp
 *
 *  Created on: Oct 17, 2016
 *      Author: storrellas
 */

#include <wcore/logging/Logger.h>

#include "RequestTraceNodes.h"

namespace wc = delfos::core;
namespace wcm = delfos::core::model;

RequestTraceNodes::RequestTraceNodes(delfosErrorHandler* err_handler) :
    _err_handler(err_handler),
    _object_nodes(nullptr)
{
}

RequestTraceNodes::~RequestTraceNodes()
{
  delete _object_nodes;
}

void RequestTraceNodes::init(delfosErrorHandler* err_handler, const delfos::core::cache_system& cache_system,
                                         size_t max_retries,
                                         time_t retry_interval,
                                         const wcore::SmartModel::DAIConf& dai_conf,
                                         const wcore::SmartModel::RedisConf& redis_conf)
{
  _object_nodes = wc::FactoryNodes::GetInstance(cache_system);
  wcm::RequestTrace r;
  _object_nodes->init(0, err_handler, "RequestTrace", nullptr, max_retries, retry_interval, &r, &dai_conf, &redis_conf);
}








