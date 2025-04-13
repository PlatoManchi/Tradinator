#pragma once
#include "AsyncTask.h"

#include <memory>
#include <vector>
#include <functional>

class AsyncTaskManager;

class SerialAsyncTask : public AsyncTask
{
public:
	SerialAsyncTask(std::shared_ptr<AsyncTaskManager> async_task_manager, std::function<void()> callback);
	SerialAsyncTask(std::shared_ptr<AsyncTaskManager> async_task_manager, std::vector<std::unique_ptr<AsyncTask>>&& tasks, std::function<void()> callback);

	// copy and move sementics
	SerialAsyncTask(const SerialAsyncTask& other) = default;
	SerialAsyncTask(SerialAsyncTask&& other) noexcept = default;
	SerialAsyncTask& operator = (const SerialAsyncTask& other) = default;
	SerialAsyncTask& operator = (SerialAsyncTask&& other) noexcept = default;

	void AddTask(std::unique_ptr<AsyncTask>&& task);
	void AddTask(std::vector<std::unique_ptr<AsyncTask>>&& tasks);

protected:
	virtual void StartTask() override;
	virtual void Update() override;
	virtual void Shutdown() override;

	void StartTaskAt(size_t index);
	void OnChildAsyncTaskComplete();

private:
	std::shared_ptr<AsyncTaskManager> m_async_task_manager;
	std::vector<std::unique_ptr<AsyncTask>> m_tasks;

	// Save task's callbacks here and replace task's callback with OnChildAsyncTaskComplete
	std::vector<std::function<void()>> m_tasks_callback_cache;

	size_t m_tasks_completed_count;
	bool m_first_task_started;
};

