#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <type_traits>

class AsyncTask
{
public:
	// default single parameter task
	AsyncTask(std::string human_readable_description, std::function<void()> callback) 
		: m_is_complete(false)
		, m_human_readable_description(human_readable_description)
		, m_callback(callback) {};

	// First function is the callback 
	template<typename ... WorkType>
	AsyncTask(std::string human_readable_description, std::function<void()> callback, WorkType ... workers);

	void StartTask();
	bool Update();
	void TaskCompleted();

	virtual ~AsyncTask();

	inline std::future_status Status() const { return m_work_future.wait_for(std::chrono::seconds(0)); };


protected:
	virtual std::string GetHumanReadableDescription() const;

	bool m_is_complete;
	std::future<void> m_work_future;

	std::string m_human_readable_description;
	std::function<void()> m_callback;
	std::vector<std::function<void()>> m_worker_list;

	// 
	std::chrono::steady_clock::time_point m_start;
};



template<typename ... WorkType>
AsyncTask::AsyncTask(std::string human_readable_description, std::function<void()> callback, WorkType ... workers)
	: m_is_complete(false)
	, m_human_readable_description(human_readable_description)
	, m_callback(callback)
{
	static_assert((std::is_same_v<WorkType, std::function<void()>> && ...), "Workers should be std::function<void()>");

	m_callback = callback;
	
	for (std::function<void()> const worker : {workers...}) {
		m_worker_list.push_back(worker);
	}

	// Reverse so that instead of 1, 2, 3 it will be 3, 2, 1
	// then we can just pop the last worker to process instead of removing from begining
	std::reverse(m_worker_list.begin(), m_worker_list.end());
}
