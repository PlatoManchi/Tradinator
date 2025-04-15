#pragma once

#include <memory>

class TradinatorCore;

class StatusBar
{
public:

	void Init(std::shared_ptr<TradinatorCore> tradinator_core);
	void Begin();
	void Show();
	void Shutdown();

private:
	std::shared_ptr<TradinatorCore> m_tradinator_core;
};

