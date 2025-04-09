#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <mutex>

#include "AsyncTask.h"

class AsyncTask;

class ThreadManager
{
public:
	ThreadManager(std::thread::id tradinator_core_thread_id);

	// taking ownership of task
	void AddTask(std::unique_ptr<AsyncTask>&& task);

	void Update();

private:
	std::mutex m_mutex;

	std::vector<std::unique_ptr<AsyncTask>> m_tasks;


	// The ID of thread TradinatorCore is constructed on.
	// Caching and using this to make debugging easy
	const std::thread::id m_tradinator_core_thread_id;
};

