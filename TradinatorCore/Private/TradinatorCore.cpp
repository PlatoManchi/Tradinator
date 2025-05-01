#include "TradinatorCore.h"

#include <thread>
#include <functional>


TradinatorCore::TradinatorCore(std::string data_folder_path)
	: m_tradinator_core_thread(std::make_shared<TradinatorCoreThread>(data_folder_path))
	, m_is_shutting_down(false)
{
	// Core is going to run as a detached thread from main thread on its own.
	// TradinatorCore class is used to interface outside world into core thread.
	std::function<void()> tradinator_core_thread_update_function = std::bind(&TradinatorCoreThread::Update, m_tradinator_core_thread);

	std::thread tradinator_core_thread(tradinator_core_thread_update_function);
	tradinator_core_thread.detach();
}


void TradinatorCore::AddMarket(std::shared_ptr<Market>&& market)
{
	m_tradinator_core_thread->AddMarket(std::move(market));
}

void TradinatorCore::Init()
{
	m_tradinator_core_thread->Init();
}

TradinatorCore::~TradinatorCore()
{
	if (!m_is_shutting_down)
	{
		Shutdown();
	}
}

void TradinatorCore::Shutdown()
{
	m_is_shutting_down = true;

	m_tradinator_core_thread->Shutdown();

	bool can_safely_shutdown = false;

	// Block closing the application if it cannot safely shutdown to prevent files from corruption.
	while (!can_safely_shutdown)
	{
		can_safely_shutdown = m_tradinator_core_thread->CanSafelyShutdown();

		// check 4 times a sec
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
}