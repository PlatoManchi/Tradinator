#include "Utils/ParallelAsyncTask.h"

#include <iostream>

#include "Utils/AsyncTaskManager.h"
#include "Utils/Log.h"

ParallelAsyncTask::ParallelAsyncTask(std::string description, std::shared_ptr<AsyncTaskManager> async_task_manager, std::function<void()> callback, size_t max_parallel_tasks)
	: AsyncTask(callback)
	, m_async_task_manager(async_task_manager)
	, m_max_parallel_tasks(max_parallel_tasks)
	, m_first_task_started(false)
	, m_tasks_started_count(0)
	, m_tasks_completed_count(0)
{
	m_human_readable_description = description;
}

ParallelAsyncTask::ParallelAsyncTask(std::string description, std::shared_ptr<AsyncTaskManager> async_task_manager, std::vector<std::unique_ptr<AsyncTask>>&& tasks, std::function<void()> callback, size_t max_parallel_tasks)
	: ParallelAsyncTask(description, async_task_manager, callback, max_parallel_tasks)
{
	AddTask(std::move(tasks));
}

void ParallelAsyncTask::AddTask(std::unique_ptr<AsyncTask>&& task)
{
	// Cache task's callback and replace it with our own callback
	size_t index = m_tasks_callback_cache.size();
	m_tasks_callback_cache.push_back(task->m_callback);
	task->m_callback = std::bind(&ParallelAsyncTask::OnChildAsyncTaskComplete, this, index);

	m_tasks.push_back(std::move(task));
}

void ParallelAsyncTask::AddTask(std::vector<std::unique_ptr<AsyncTask>>&& tasks)
{
	for (std::unique_ptr<AsyncTask>& task : tasks)
	{
		AddTask(std::move(task));
	}
}

void ParallelAsyncTask::StartTask()
{
	Log::GetInstance().Write(std::format("{} ...", GetHumanReadableDescription()));

	m_start = std::chrono::steady_clock::now();

	m_first_task_started = false;

	// Calling UpdateTask here will call AddTask on async_thread_manager which has a mutex
	// locked already this ParallelAsyncTask. Locking already locked mutex is undefined behavior.
	// So just process in update.
	//StartFirstBatch();
}

void ParallelAsyncTask::Update()
{
	if (!m_first_task_started)
	{
		m_first_task_started = true;

		if (m_tasks.size() == 0 || m_is_shut_down)
		{
			TaskCompleted();

			return;
		}
		else
		{
			StartFirstBatch();
		}
	}
}

void ParallelAsyncTask::StartFirstBatch()
{
	if (m_is_shut_down)
	{
		TaskCompleted();
		return;
	}
	
	size_t num_tasks_to_start = m_max_parallel_tasks > m_tasks.size() ? m_tasks.size() : m_max_parallel_tasks;
	
	for (int i = 0; i < num_tasks_to_start; ++i)
	{
		m_async_task_manager->AddTask(std::move(m_tasks[i]));
	}

	m_tasks_started_count = num_tasks_to_start;
}

void ParallelAsyncTask::OnChildAsyncTaskComplete(size_t index)
{
	m_tasks_callback_cache[index]();

	m_tasks_completed_count++;

	if (m_tasks_completed_count >= m_tasks.size())
	{
		TaskCompleted();
	}
	else if(m_tasks_started_count < m_tasks.size())
	{
		m_async_task_manager->AddTask(std::move(m_tasks[m_tasks_started_count]));
		m_tasks_started_count++;
	}
}

void ParallelAsyncTask::Shutdown()
{
	AsyncTask::Shutdown();

	m_tasks.clear();

	// Parallel task doesn't have anything to do in itself. So it can finish off immediately when shutting down
	TaskCompleted();
}