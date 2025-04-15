#include "Utils/SerialAsyncTask.h"

#include "Utils/AsyncTaskManager.h"
#include "Utils/Log.h"

SerialAsyncTask::SerialAsyncTask(std::string description, std::shared_ptr<AsyncTaskManager> async_task_manager, std::function<void()> callback)
	: AsyncTask(callback)
	, m_async_task_manager(async_task_manager)
	, m_tasks_completed_count(0)
	, m_first_task_started(false)
{
	m_human_readable_description = description;
}

SerialAsyncTask::SerialAsyncTask(std::string description, std::shared_ptr<AsyncTaskManager> async_task_manager, std::vector<std::unique_ptr<AsyncTask>>&& tasks, std::function<void()> callback)
	: SerialAsyncTask(description, async_task_manager, callback)
{
	AddTask(std::move(tasks));
}

void SerialAsyncTask::AddTask(std::unique_ptr<AsyncTask>&& task)
{
	// Cache task's callback and replace it with our own callback
	m_tasks_callback_cache.push_back(task->m_callback);
	task->m_callback = std::bind(&SerialAsyncTask::OnChildAsyncTaskComplete, this);

	m_tasks.push_back(std::move(task));
}

void SerialAsyncTask::AddTask(std::vector<std::unique_ptr<AsyncTask>>&& tasks)
{
	for (std::unique_ptr<AsyncTask>& task : tasks)
	{
		AddTask(std::move(task));
	}
}

void SerialAsyncTask::StartTask()
{
	Log::GetInstance().Write(std::format("{} ...", GetHumanReadableDescription()));

	m_start = std::chrono::steady_clock::now();

	if (m_tasks.size() == 0 || m_is_shut_down)
	{
		TaskCompleted();

		return;
	}

	m_first_task_started = false;
}

void SerialAsyncTask::StartTaskAt(size_t index)
{
	m_async_task_manager->AddTask(std::move(m_tasks[index]));
}

void SerialAsyncTask::OnChildAsyncTaskComplete()
{
	m_tasks_callback_cache[m_tasks_completed_count]();
	
	m_tasks_completed_count++;

	if (m_tasks_completed_count < m_tasks.size())
	{
		StartTaskAt(m_tasks_completed_count);
	}
	else
	{
		TaskCompleted();
	}
}

void SerialAsyncTask::Update()
{
	if (!m_first_task_started)
	{
		m_first_task_started = true;
		StartTaskAt(0);
	}
}

void SerialAsyncTask::Shutdown()
{
	AsyncTask::Shutdown();

	m_tasks.clear();

	// Serial task doesn't have anything to do in itself. So it can finish off immediately when shutting down
	TaskCompleted();
}
