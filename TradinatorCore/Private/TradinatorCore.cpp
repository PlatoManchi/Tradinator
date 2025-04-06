#include "TradinatorCore.h"

#include <iostream>
#include <fstream>
#include <filesystem>

#include "Market/Market.h"
#include "Utils/ThreadManager.h"



TradinatorCore::TradinatorCore(std::string data_folder_path)
	: m_thread_manager(std::make_shared<ThreadManager>())
	, m_data_folder_path(data_folder_path)
{
	std::filesystem::create_directories("Data/Raw");
	
	std::ofstream myfile;
	
	myfile.open("Data/Raw/example.txt");
	myfile << "Writing this to a file.\n";
	myfile.close();
}

void TradinatorCore::InitializeAllMarkets()
{
	for (std::shared_ptr<Market>& market : m_market_list)
	{
		market->GatherSymbols();
	}
}

void TradinatorCore::Update()
{
	m_thread_manager->Update();

	for (std::shared_ptr<Market>& market : m_market_list)
	{
		market->Update();
	}
}

void TradinatorCore::AddMarket(std::shared_ptr<Market>&& market)
{
	m_market_list.emplace_back(market);
}