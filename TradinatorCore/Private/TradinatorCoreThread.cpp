#include "TradinatorCoreThread.h"

#include <thread>

#include "Market/Market.h"
#include "Utils/AsyncTaskManager.h"

TradinatorCoreThread::TradinatorCoreThread(std::string data_folder_path)
	: m_data_folder_path(data_folder_path)
	, m_async_task_manager(std::make_shared<AsyncTaskManager>())
	, m_is_shut_down(false)
{

}

void TradinatorCoreThread::AddMarket(std::shared_ptr<Market>&& market)
{
	std::shared_ptr<Market>& stored_market = m_market_list.emplace_back(market);
	stored_market->SetOwningTradinatorCoreThread(this->weak_from_this());
	stored_market->Init();
}

void TradinatorCoreThread::Update()
{
	// meat and potatoes
	while (true)
	{
		if (m_is_shut_down)
		{
			bool can_safely_shut_down = m_async_task_manager.get() && m_async_task_manager->CanSafelyShutDown();

			if (can_safely_shut_down)
			{
				// If thread can safely shutdown, then break out of loop.
				break;
			}
		}

		m_async_task_manager->Update();

		// Update 100 times in a second
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

bool TradinatorCoreThread::CanSafelyShutdown() const
{
	return m_async_task_manager.get() && m_async_task_manager->CanSafelyShutDown();
}

void TradinatorCoreThread::Shutdown()
{
	m_is_shut_down = true;

	m_async_task_manager->Shutdown();
}