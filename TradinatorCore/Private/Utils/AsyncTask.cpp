#include<iostream>

#include "Utils/AsyncTask.h"
#include "Utils/Log.h"


void AsyncTask::AddWork(std::function<void()> work)
{
	m_worker_list.push_back(work);
}

void AsyncTask::StartTask()
{
	if (!GetHumanReadableDescription().empty())
		Log::GetInstance().Write(std::format("{} ...", GetHumanReadableDescription()));
	
	m_start = std::chrono::steady_clock::now();
	m_worker_index = 0;

	if (!m_is_shut_down && m_worker_list.size() > 0)
	{
		m_work_future = std::async(std::launch::async, std::bind(&AsyncTask::DoWork, this));
	}
	else
	{
		// if there are no workers then just call callback
		TaskCompleted();
	}
}

void AsyncTask::DoWork()
{
	while (!m_is_shut_down && m_worker_index < m_worker_list.size())
	{
		m_worker_list[m_worker_index]();
		m_worker_index++;
	}

	// Don't call TaskCompleted here because DoWork is on another thread.
	// We want all callbacks to be on main TradinatorCoreThread thread because the callbacks might
	// do add more tasks or do something like read/write
}

void AsyncTask::Update()
{
	if (!m_is_complete && m_work_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		TaskCompleted();
	}
}

void AsyncTask::TaskCompleted()
{
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	if (!GetHumanReadableDescription().empty())
		Log::GetInstance().Write(std::format("{} completed in {} sec.\n", GetHumanReadableDescription(), std::to_string(std::chrono::duration<double>(end - m_start).count())));
	
	if (!m_is_shut_down) 
	{
		m_callback();
	}

	m_is_complete = true;
}

std::string AsyncTask::GetHumanReadableDescription() const
{
	return m_human_readable_description;
}

AsyncTask::~AsyncTask()
{
	m_is_complete = true;
}

void AsyncTask::Shutdown()
{
	// remove all the worker list which will cause for callback to be called when current future completes;
	m_is_shut_down = true;
	m_worker_list.clear();
}