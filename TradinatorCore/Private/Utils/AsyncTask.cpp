#include<iostream>

#include "Utils/AsyncTask.h"



void AsyncTask::StartTask()
{
	if (m_worker_list.size() > 0)
	{
		m_work_future = std::async(std::launch::async, m_worker_list[m_worker_list.size() - 1]);
		m_worker_list.pop_back();
	}
	else
	{
		// if there are no workers then just call callback
		TaskCompleted();
	}
}

bool AsyncTask::Update()
{
	if (!m_is_complete && m_work_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		if (m_worker_list.size() > 0)
		{
			m_work_future = std::async(std::launch::async, m_worker_list[m_worker_list.size() - 1]);
			m_worker_list.pop_back();
		}
		else
		{
			TaskCompleted();
		}
	}

	return !m_is_complete;
}

void AsyncTask::TaskCompleted()
{
	m_is_complete = true;
	m_callback();
}


AsyncTask::~AsyncTask()
{
	m_is_complete = true;
}