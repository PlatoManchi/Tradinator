#pragma once

#include <memory>
#include <string>


class Security;

class SecurityWindow
{
public:
	SecurityWindow(std::shared_ptr<Security> security);
	void Show();

private:
	std::string m_cached_label_id;
	std::shared_ptr<Security> m_security;
};

