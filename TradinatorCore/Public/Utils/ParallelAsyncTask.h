#pragma once

#include "AsyncTask.h"

#include <memory>
#include <vector>

class AsyncTaskManager;

class ParallelAsyncTask : public AsyncTask
{
public:
	ParallelAsyncTask(std::string description, std::shared_ptr<AsyncTaskManager> async_task_manager, std::function<void()> callback, size_t max_parallel_tasks = 32);
	ParallelAsyncTask(std::string description, std::shared_ptr<AsyncTaskManager> async_task_manager, std::vector<std::unique_ptr<AsyncTask>>&& tasks, std::function<void()> callback, size_t max_parallel_tasks = 32);

	// copy and move sementics
	ParallelAsyncTask(const ParallelAsyncTask& other) = default;
	ParallelAsyncTask(ParallelAsyncTask&& other) noexcept = default;
	ParallelAsyncTask& operator = (const ParallelAsyncTask& other) = default;
	ParallelAsyncTask& operator = (ParallelAsyncTask&& other) noexcept = default;

	void AddTask(std::unique_ptr<AsyncTask>&& task);
	void AddTasks(std::vector<std::unique_ptr<AsyncTask>>&& task);

	template<typename Task, typename ... Tasks>
	ParallelAsyncTask(std::string description
		, std::shared_ptr<AsyncTaskManager> async_task_manager
		, std::unique_ptr<Task>&& task
		, Tasks ... tasks_and_callback) requires std::convertible_to<Task, AsyncTask>
		: ParallelAsyncTask(std::move(tasks_and_callback) ...)
	{
		assert(task);

		m_human_readable_description = description;
		m_tasks_completed_count = 0;
		m_first_task_started = false;
		m_async_task_manager = async_task_manager;

		AddTask(std::move(task));

		std::reverse(m_tasks.begin(), m_tasks.end());
		std::reverse(m_tasks_callback_cache.begin(), m_tasks_callback_cache.end());
	}

protected:
	template<typename Task, typename ... Tasks>
	ParallelAsyncTask(std::unique_ptr<Task>&& task, Tasks ... tasks) requires std::convertible_to<Task, AsyncTask>
		: ParallelAsyncTask(std::move(tasks) ...)
	{
		assert(task);

		AddTask(std::move(task));
	}

	template<typename Callback>
	ParallelAsyncTask(Callback callback) requires std::convertible_to<Callback, std::function<void()>>
	{
		m_callback = callback;
	}

	virtual void StartTask() override;
	virtual void Update() override;
	virtual void Shutdown() override;
	
	void StartFirstBatch();
	void OnChildAsyncTaskComplete(size_t index);

private:
	size_t m_max_parallel_tasks;
	std::shared_ptr<AsyncTaskManager> m_async_task_manager;
	std::vector<std::unique_ptr<AsyncTask>> m_tasks;

	// Save task's callbacks here and replace task's callback with OnChildAsyncTaskComplete
	std::vector<std::function<void()>> m_tasks_callback_cache;

	bool m_first_task_started;
	size_t m_tasks_started_count;
	size_t m_tasks_completed_count;
};

