#include<iostream>

#include "Utils/AsyncTask.h"



void AsyncTask::StartTask()
{
	std::cout << GetHumanReadableDescription() << " ... " << std::endl;

	m_start = std::chrono::steady_clock::now();

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

void AsyncTask::Update()
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
}

void AsyncTask::TaskCompleted()
{
	m_is_complete = true;

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << GetHumanReadableDescription() << " completed in " << std::to_string(std::chrono::duration<double>(end - m_start).count()) << " sec." << std::endl << std::endl << std::endl;

	m_callback();
}

std::string AsyncTask::GetHumanReadableDescription() const
{
	return m_human_readable_description;
}

AsyncTask::~AsyncTask()
{
	m_is_complete = true;
}