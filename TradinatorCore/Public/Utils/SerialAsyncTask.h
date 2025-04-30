#pragma once
#include "AsyncTask.h"

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <string>
#include <type_traits>

class AsyncTaskManager;


class SerialAsyncTask : public AsyncTask
{
public:
	SerialAsyncTask(std::string description, std::shared_ptr<AsyncTaskManager> async_task_manager, std::function<void()> callback);
	SerialAsyncTask(std::string description, std::shared_ptr<AsyncTaskManager> async_task_manager, std::vector<std::unique_ptr<AsyncTask>>&& tasks, std::function<void()> callback);


	void AddTask(std::unique_ptr<AsyncTask>&& task);
	void AddTasks(std::vector<std::unique_ptr<AsyncTask>>&& tasks);

	template<typename Task, typename ... Tasks>
	SerialAsyncTask(std::string description
		, std::shared_ptr<AsyncTaskManager> async_task_manager
		, std::unique_ptr<Task>&& task
		, Tasks ... tasks_and_callback) requires std::convertible_to<Task, AsyncTask>
		: SerialAsyncTask(std::move(tasks_and_callback) ...)
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

	// copy and move sementics
	SerialAsyncTask(const SerialAsyncTask& other) = default;
	SerialAsyncTask(SerialAsyncTask&& other) noexcept = default;
	SerialAsyncTask& operator = (const SerialAsyncTask& other) = default;
	SerialAsyncTask& operator = (SerialAsyncTask&& other) noexcept = default;


protected:
	template<typename Task, typename ... Tasks>
	SerialAsyncTask(std::unique_ptr<Task>&& task, Tasks ... tasks) requires std::convertible_to<Task, AsyncTask>
		: SerialAsyncTask(std::move(tasks) ...)
	{
		assert(task);

		AddTask(std::move(task));
	}

	template<typename Callback>
	SerialAsyncTask(Callback callback) requires std::convertible_to<Callback, std::function<void()>>
	{
		m_callback = callback;
	}


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

