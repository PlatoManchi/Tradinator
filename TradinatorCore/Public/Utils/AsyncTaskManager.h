#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <mutex>

#include "AsyncTask.h"

class AsyncTask;

class AsyncTaskManager
{
public:
	AsyncTaskManager();

	// taking ownership of task
	void AddTask(std::unique_ptr<AsyncTask>&& task);

	void Update();
	void Shutdown();

	inline bool CanSafelyShutDown() const { return m_is_shutting_down && (m_tasks.size() == 0); };

private:
	std::mutex m_mutex;

	std::vector<std::unique_ptr<AsyncTask>> m_tasks;

	bool m_is_shutting_down;
};

