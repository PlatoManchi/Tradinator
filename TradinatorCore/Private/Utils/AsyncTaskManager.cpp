#include "Utils/AsyncTaskManager.h"

#include "Utils/AsyncTask.h"


AsyncTaskManager::AsyncTaskManager()
	: m_is_shutting_down(false)
{

}

void AsyncTaskManager::AddTask(std::unique_ptr<AsyncTask>&& task)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	
	task->StartTask();
	m_add_tasks_buffer.push_back(std::move(task));
}

void AsyncTaskManager::Update()
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		
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
}

void AsyncTaskManager::Shutdown()
{
	for (std::unique_ptr<AsyncTask>& task : m_tasks)
	{
		task->Shutdown();
	}

	m_is_shutting_down = true;
}