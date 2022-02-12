#include <iostream>
#include "PeFile.hpp"
#include "gui/Renderer.h"
#include "Logger.h"
#include <imgui_internal.h>

static PeReaderParser currentParsedFile = PeReaderParser();

int main()
{
	LogInfo("Welcome!");


	MainConfig.windowToggles.showFileInformations = false;
	

	g_MainRender.Initialize();
	while (g_MainRender.NeedsToRender())
	{
		auto displaySize = ImGui::GetIO().DisplaySize;
		g_MainRender.BeginRender();

		// main tab bar
		{
			ImGui::BeginMainMenuBar();

			if(ImGui::BeginMenu("File"))
			{
				
				if (ImGui::MenuItem("Open", "", false, !currentParsedFile.hasAFile))
				{
					char filename[MAX_PATH] = { 0 };
				
					OPENFILENAMEA ofn;
					ZeroMemory(&filename, sizeof(filename));
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hwnd;
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrTitle = "Select a File";
					ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

					if (GetOpenFileNameA(&ofn))
					{
						LogInfo("Parsing %s", filename);
						currentParsedFile.Initialize(filename);
						if (currentParsedFile.Parse() == true)
						{
							LogInfo("Quick Informations:");
							LogInfo("Virtual Size: 0x%llx", currentParsedFile.GetParser()->GetNtHeaders()->OptionalHeader.SizeOfImage);
							LogInfo("Section Count: %i", currentParsedFile.GetParser()->GetSectionNumbers());
							for (int idx = 0; idx < currentParsedFile.GetParser()->GetSectionNumbers(); idx++)
							{
								auto cSection = currentParsedFile.GetParser()->GetSectionHeaderByIndex(idx);

								LogInfo("\t%.*s; Disk Size: 0x%x; Virtual Size: 0x%x; Virtual Offset: 0x%x", 8, cSection->Name, cSection->SizeOfRawData, cSection->Misc.VirtualSize, cSection->VirtualAddress);
							}
						}
						else
						{
							LogError("You can only input " NT_ONLY_STR " pe file's! Or you need to download the other version from the tool!");
						}
					}
				}

				if (ImGui::MenuItem("Close", "", false, currentParsedFile.hasAFile))
				{
					currentParsedFile.Release();
				}

				ImGui::EndMenu();
			}

			
			if (ImGui::BeginMenu("Windows"))
			{
				ImGui::MenuItem("PE Infos (PE Bear inspired)", "", &MainConfig.windowToggles.showFileInformations);

				ImGui::ItemSize(ImVec2(0, 5));

				if (ImGui::MenuItem("Clear Console")) g_MainLogger.ClearConsole();

				ImGui::EndMenu();
			}
			

			ImGui::EndMainMenuBar();
		}
		if (currentParsedFile.hasAFile)
		{
			if (MainConfig.windowToggles.showFileInformations)
			{
				if (ImGui::Begin("PE Infos (PE Bear inspired)", 0))
				{
					static short selectedTab = -1;

#define MAKE_SELECTABLE(name, idx, tabName) if(ImGui::Selectable(name, selectedTab == idx)) { \
					selectedTab = idx;}

					auto windowSize = ImGui::GetWindowSize();
					ImGui::BeginChild("##hdrDisplay", ImVec2(windowSize.x / 4, -1));

					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7.f);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7.f);

					ImGui::BeginGroup();
					
					ImGui::Selectable(currentParsedFile.absName);
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7.f);
					ImGui::BeginGroup();

					MAKE_SELECTABLE("DOS Header's", 0, "DOS Header's");
					MAKE_SELECTABLE("NT Header's", 1, "NT Header's");
					ImGui::ItemSize(ImVec2(0, 0));
					MAKE_SELECTABLE("Section's", 2, "NT Header's");

					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7.f);
					ImGui::BeginGroup();
					{
						for (int idx = 0; idx < currentParsedFile.GetParser()->GetSectionNumbers(); idx++)
						{
							auto cSection = currentParsedFile.GetParser()->GetSectionHeaderByIndex(idx);
							char buf[32] = { 0 };
							sprintf_s(buf, "%.*s", 8, cSection->Name);

							if (ImGui::Selectable(buf)) { selectedTab = 2; }
						}
					}
					ImGui::EndGroup();

					ImGui::EndGroup();
					ImGui::EndGroup();

					ImGui::EndChild();

					ImGui::SameLine();

					ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
					ImGui::BeginChild("##conDisplay", ImVec2(-1, -1));
					
					ImGui::BeginTabBar("##conTabBar");
					if (ImGui::BeginTabItem("DOS Header's"))
					{
						
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("NT Header's"))
					{

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Section Header's"))
					{
						
						if (ImGui::BeginTable("##section_table", 9, ImGuiTableFlags_Resizable | ImGuiTableFlags_PadOuterX))
						{
							ImGui::PushFont(g_MainRender.fontMap[FONT_BAHNSCHRIFT_TINY]);
							ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Name");
							ImGui::TableNextColumn();
							ImGui::Text("Raw Addr.");
							ImGui::TableNextColumn();
							ImGui::Text("Raw Size.");
							ImGui::TableNextColumn();
							ImGui::Text("Virt Addr.");
							ImGui::TableNextColumn();
							ImGui::Text("Virt Size.");
							ImGui::TableNextColumn();
							ImGui::Text("Characteristics");
							ImGui::TableNextColumn();
							ImGui::Text("Ptr to Reloc");
							ImGui::TableNextColumn();
							ImGui::Text("Num. of Reloc.");
							ImGui::TableNextColumn();
							ImGui::Text("Num. of Linenumber");
							

							

							for (int idx = 0; idx < currentParsedFile.GetParser()->GetSectionNumbers(); idx++)
							{
								ImGui::TableNextRow();
								auto cSection = currentParsedFile.GetParser()->GetSectionHeaderByIndex(idx);
								ImGui::TableSetColumnIndex(0);
								ImGui::Text("%.*s", 8, cSection->Name);
								ImGui::TableNextColumn();
								ImGui::Text("%X", cSection->PointerToRawData);
								ImGui::TableNextColumn();
								ImGui::Text("%X", cSection->SizeOfRawData);
								ImGui::TableNextColumn();
								ImGui::Text("%X", cSection->VirtualAddress);
								ImGui::TableNextColumn();
								ImGui::Text("%X", cSection->Misc.VirtualSize);
							}

							ImGui::PopFont();
							ImGui::EndTable();
						}

						ImGui::EndTabItem();
					}

					ImGui::EndTabBar();

					if (selectedTab != -1)
					{
						ImGuiContext& g = *GImGui;
						ImGuiWindow* window = g.CurrentWindow;
						ImGuiID id = window->GetID("##conTabBar");
						ImGuiTabBar* tab_bar = g.TabBars.GetOrAddByKey(id);

						auto swTab = tab_bar->Tabs[selectedTab];
						tab_bar->NextSelectedTabId = swTab.ID;

						selectedTab = -1;
					}

					ImGui::EndChild();
					ImGui::PopStyleColor();
				}
				ImGui::End();
			}
		}
		// down bar
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(31, 33, 38).Value);

			ImVec2 windowLogSize = {displaySize.x, 200};


			if (ImGui::Begin("##Log", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
			{
				g_MainLogger.Render(g_MainRender.fontMap[FONT_BAHNSCHRIFT_NORMAL]);
			}

			auto inVSize = ImVec2(); inVSize.x = 0;
			inVSize.y = displaySize.y - windowLogSize.y;
			ImGui::SetWindowPos(inVSize);

			{
				auto sizeBefore = windowLogSize;
				ImGui::SetWindowSize(sizeBefore);
			}

			ImGui::End();
			ImGui::PopStyleColor();
		}

		g_MainRender.EndRender();
	}
	g_MainRender.UnInitialize();
	g_MainLogger.Release();
	
	//PeReaderParser reader = PeReaderParser();
	//reader.Initialize(argv[1]);
	//reader.Parse();
	//printf("State Readed: %s\n", argv[1]);
	//printf("Sections: %i\n", reader.GetParser()->GetSectionNumbers());
	//for (int i = 0; i < reader.GetParser()->GetSectionNumbers(); i++)
	//{
	//	auto currentSection = reader.GetParser()->GetSectionHeaderByIndex(i);
	//	printf("-- %s\n", currentSection->Name);
	//}

	//printf("State Mapping!\n");
	//PeMapper_Normal normalMapper = PeMapper_Normal();
	//normalMapper.Initialize(reader.GetParser());

	//normalMapper.MapPeFile();

	//printf("-- Mapped memory address: %p\n", normalMapper.GetBaseAddr());

	//while (true)
	//{
	//	if (GetAsyncKeyState(VK_INSERT) & 1) { break; }
	//}

	//printf("Running\n");
	//auto pEntry = normalMapper.GetBaseAddr() + normalMapper.GetNtHeaders()->OptionalHeader.AddressOfEntryPoint;

	//

	//typedef void(*EntryFuncDef)();

	//((EntryFuncDef)pEntry)();

	//normalMapper.Release();
	//reader.Release();
}