#pragma once
#include "BaseModules.hpp"

#define DISP_CREDIT(name, author, link) ImGui::BeginGroup(); ImGui::Text("Project: %s", name); ImGui::SameLine(); ImGui::Text("Author(s): %s", author); ImGui::EndGroup(); if(ImGui::IsItemHovered()) { ImGui::BeginTooltip();ImGui::Text("%s",link);ImGui::EndTooltip();} if(ImGui::IsItemClicked()) {ShellExecuteA(NULL, "open", link, NULL, NULL, SW_SHOWNORMAL);}

class CreditsModuleGui : public BaseModuleGui
{
public:
	const char* GetWindowName()
	{
		return "Credits";
	}

	void RenderWindow()
	{
		

		ImGui::BeginChild("##creditsBorder", ImVec2(-1, -1));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
		ImGui::BeginGroup();
		ImGui::Text("Thanks to the People that made these Projects!");
		DISP_CREDIT("ImGui - GUI Lib", "ocornut", "https://github.com/ocornut/imgui");
		DISP_CREDIT("Zydis - Asm Lib", "zyantific", "https://github.com/zyantific/zydis");
		ImGui::EndGroup();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
		ImGui::EndChild();
		
	}

};

REGISTER_MODULE(CreditsModuleGui)