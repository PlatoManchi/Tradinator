#include "Utils/ThreadManager.h"

void ThreadManager::AddTask(std::unique_ptr<AsyncTask>&& task)
{
	task->StartTask();
	m_tasks.push_back(std::move(task));
}

void ThreadManager::Update()
{
	for (std::unique_ptr<AsyncTask>& task : m_tasks)
	{
		if (!task->Update())
		{
			task.reset();	// delete completed task
		}
	}

	// remove all completed tasks
	m_tasks.erase(std::remove_if(m_tasks.begin(), m_tasks.end(),
		[](std::unique_ptr<AsyncTask>& task) {
			return task.get() == nullptr;
		}), m_tasks.end());
}