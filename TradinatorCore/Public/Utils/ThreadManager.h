#pragma once

#include <memory>
#include <vector>

#include "AsyncTask.h"

class AsyncTask;

class ThreadManager
{
public:
	// taking ownership of task
	void AddTask(std::unique_ptr<AsyncTask>&& task);

	void Update();

private:
	std::vector<std::unique_ptr<AsyncTask>> m_tasks;
};

