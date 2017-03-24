/*
 * LmSessionThreadController.h
 *
 *  Created on: Jan 18, 2016
 *      Author: mjlopez
 */

#pragma once

#include <string>

#include <wcore/utils/containers/model_structures/ThreadController.h>
#include <wcore/utils/helper/DAIHelper.h>

#include <src/model/session/containers/RequestTraceNodes.h>
#include <src/workflows/Workflow.h>
#include <src/EnzoConfiguration.h>


class RequestTraceThreadController :
		public delfos::core::ThreadController
{
private:
  RequestTraceNodes* _request_traces;
  delfos::core::DAIHelper _dai_helper;
  uint _max_request_trace_amount;
  std::list<delfos::enzo::Workflow*>& _workflow_list;
public:
  RequestTraceThreadController(delfosErrorHandler *weh,
                                RequestTraceNodes* request_traces,
                                const bool clear_request_trace,
                                std::list<delfos::enzo::Workflow*>& workflow_list);
	virtual ~RequestTraceThreadController();

	/**
	 * Perform the actions assigned periodically
	 */
	virtual void do_stuff();
};

