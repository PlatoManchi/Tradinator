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

	void AddTask(std::unique_ptr<AsyncTask>&& task);

	void Update();
	void Shutdown();
	

	inline bool IsProcessing() const { return m_tasks.size() != 0; }
	inline bool IsShuttingDown() const { return m_is_shutting_down; }
	inline bool CanSafelyShutDown() const { return m_is_shutting_down && (m_tasks.size() == 0); };

private:
	std::mutex m_async_task_manager_lock;

	std::vector<std::unique_ptr<AsyncTask>> m_tasks;
	std::vector<std::unique_ptr<AsyncTask>> m_add_tasks_buffer;

	bool m_is_shutting_down;

	// This is just for decoration purpose
	size_t m_shutting_down_remaining_tasks;
	size_t m_prev_shutting_down_remaining_tasks;
};

