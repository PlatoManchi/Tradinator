#pragma once

#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <type_traits>

class AsyncTask
{
public:
	AsyncTask(std::string human_readable_description, std::vector < std::function<void()> > workers, std::function<void()> callback)
		: m_is_complete(false)
		, m_is_shut_down(false)
		, m_human_readable_description(human_readable_description)
		, m_callback(callback)
		, m_worker_list(workers)
	{ }

	// Last function should be callback when all the work is finished
	template<typename Worker, typename ... WorkersAndCallback>
	AsyncTask(std::string human_readable_description, Worker worker, WorkersAndCallback ... workers_and_callback)
		: AsyncTask(workers_and_callback...)
	{
		m_is_complete = false;
		m_is_shut_down = false;
		m_human_readable_description = human_readable_description;
		m_worker_list.push_back(worker);
	}


	virtual ~AsyncTask();

	void Shutdown();

	inline std::future_status Status() const { return m_work_future.wait_for(std::chrono::seconds(0)); };


protected:
	template<typename Worker, typename ... Workers>
	AsyncTask(Worker worker, Workers ... workers)
		: AsyncTask(workers...)
	{
		m_worker_list.push_back(worker);
	}

	template<typename Callback>
	AsyncTask(Callback callback)
		: m_is_complete(false)
		, m_is_shut_down(false)
		, m_callback(callback)
	{
	}

	// Used by subclasses if they want to take params other than std::function in constructor
	AsyncTask()
		: m_is_complete (false)
		, m_is_shut_down(false)
	{
		m_human_readable_description = "";
	};


	virtual std::string GetHumanReadableDescription() const;

	bool m_is_complete;
	bool m_is_shut_down;
	std::future<void> m_work_future;

	std::string m_human_readable_description;
	std::function<void()> m_callback;
	std::vector<std::function<void()>> m_worker_list;

	// 
	std::chrono::steady_clock::time_point m_start;

private:

	void StartTask();
	void Update();
	void TaskCompleted();


	friend class ThreadManager;
};
