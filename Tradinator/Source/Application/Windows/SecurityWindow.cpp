#include "SecurityWindow.h"

#include "imgui.h"

#include "Data/Security.h"

SecurityWindow::SecurityWindow(std::shared_ptr<Security> security)
	: m_security(security)
{
	m_cached_label_id = m_security->Name() + "##" +m_security->ISIN_Number();
}

void SecurityWindow::Show()
{
	
	if (ImGui::Begin(m_cached_label_id.c_str()))
	{

	}
	ImGui::End();
}