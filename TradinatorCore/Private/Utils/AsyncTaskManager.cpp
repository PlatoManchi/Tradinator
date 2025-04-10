#include "Utils/AsyncTaskManager.h"

#include "Utils/AsyncTask.h"


AsyncTaskManager::AsyncTaskManager()
	: m_is_shutting_down(false)
{

}

void AsyncTaskManager::AddTask(std::unique_ptr<AsyncTask>&& task)
{
	m_mutex.lock();

	task->StartTask();
	m_tasks.push_back(std::move(task));

	m_mutex.unlock();
}

void AsyncTaskManager::Update()
{
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