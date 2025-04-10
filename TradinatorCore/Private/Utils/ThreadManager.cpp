#include "Utils/ThreadManager.h"

#include "Utils/AsyncTask.h"


ThreadManager::ThreadManager(std::thread::id tradinator_core_thread_id)
	: m_tradinator_core_thread_id(tradinator_core_thread_id)
{

}

void ThreadManager::AddTask(std::unique_ptr<AsyncTask>&& task)
{
	m_mutex.lock();

	task->StartTask();
	m_tasks.push_back(std::move(task));

	m_mutex.unlock();
}

void ThreadManager::Update()
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

void ThreadManager::Shutdown()
{
	for (std::unique_ptr<AsyncTask>& task : m_tasks)
	{
		task->Shutdown();
	}
}