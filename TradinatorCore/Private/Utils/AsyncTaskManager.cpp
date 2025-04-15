#include "Utils/AsyncTaskManager.h"

#include "Utils/AsyncTask.h"
#include "Utils/Log.h"


AsyncTaskManager::AsyncTaskManager()
	: m_is_shutting_down(false)
	, m_shutting_down_remaining_tasks(0)
{

}

void AsyncTaskManager::AddTask(std::unique_ptr<AsyncTask>&& task)
{
	std::lock_guard<std::mutex> lock(m_async_task_manager_lock);
	
	task->StartTask();
	m_add_tasks_buffer.push_back(std::move(task));
}

void AsyncTaskManager::Update()
{
	{
		std::lock_guard<std::mutex> lock(m_async_task_manager_lock);
		
		for (std::unique_ptr<AsyncTask>& task_to_add : m_add_tasks_buffer)
		{
			m_tasks.push_back(std::move(task_to_add));
		}

		// clear buffer
		m_add_tasks_buffer.clear();
	}

	for (std::unique_ptr<AsyncTask>& task : m_tasks)
	{
		task->Update();
	}

	// remove all completed tasks
	m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
		[](std::unique_ptr<AsyncTask>& task) {
			return task->m_is_complete;
		}), m_tasks.end());

	if (m_is_shutting_down)
	{
		if (m_tasks.size() != m_prev_shutting_down_remaining_tasks)
		{
			m_prev_shutting_down_remaining_tasks = m_tasks.size();
			Log::GetInstance().Write(std::format("Finishing up. Remaining tasks: {}/{} ...", m_tasks.size(), m_shutting_down_remaining_tasks));
			if (m_shutting_down_remaining_tasks < m_tasks.size()) {
				m_shutting_down_remaining_tasks = m_tasks.size();
			}
		}
	}
}

void AsyncTaskManager::Shutdown()
{
	std::lock_guard<std::mutex> lock(m_async_task_manager_lock);

	m_is_shutting_down = true;

	for (std::unique_ptr<AsyncTask>& task : m_tasks)
	{
		task->Shutdown();
	}
	for (std::unique_ptr<AsyncTask>& task : m_add_tasks_buffer)
	{
		task->Shutdown();
	}
	
	m_shutting_down_remaining_tasks = m_tasks.size();
	m_prev_shutting_down_remaining_tasks = 0;
}