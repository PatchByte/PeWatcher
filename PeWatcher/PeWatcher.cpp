#include <iostream>
#include "PeFile.hpp"
#include "gui/Renderer.h"
#include "Logger.h"
#include <imgui_internal.h>

#include "modules/BaseModules.hpp"
#include "modules/PeInfo.hpp"

int main()
{
	LogInfo("Welcome!");

	MainConfig.hasMapped = false;

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
							LogInfo("Virtual Size: 0x%X", currentParsedFile.GetParser()->GetNtHeaders()->OptionalHeader.SizeOfImage);
							LogInfo("Section Count: %i", currentParsedFile.GetParser()->GetSectionNumbers());
							for (int idx = 0; idx < currentParsedFile.GetParser()->GetSectionNumbers(); idx++)
							{
								auto cSection = currentParsedFile.GetParser()->GetSectionHeaderByIndex(idx);

								LogInfo("\t%.*s; Disk Size: 0x%x; Virtual Size: 0x%x; Virtual Offset: 0x%x", 8, cSection->Name, cSection->SizeOfRawData, cSection->Misc.VirtualSize, cSection->VirtualAddress);
								MainConfig.hasMapped = false;
							}
						}
						else
						{
							LogError("You can only input " NT_ONLY_STR " pe file's! Or you need to download the other version from the tool!");
						}
					}
				}

				if (ImGui::MenuItem("Simulate Mapping", "", false, currentParsedFile.hasAFile && (MainConfig.hasMapped == false)))
				{
					currentParsedFile.GetNormalMapper()->Initialize(currentParsedFile.GetParser());
					if (currentParsedFile.GetNormalMapper()->MapPeFile() == true)
					{
						LogInfo("Mapped Pe File!");
						MainConfig.hasMapped = true;
					}
					else
					{
						LogError("Mapping failed Pe File!");
						MainConfig.hasMapped = false;
						currentParsedFile.GetNormalMapper()->Release();
					}
				}

				if (ImGui::MenuItem("Close", "", false, currentParsedFile.hasAFile))
				{
					if (MainConfig.hasMapped == true)
					{
						MainConfig.hasMapped = false;
						currentParsedFile.GetNormalMapper()->Release();
					}
					
					currentParsedFile.Release();
				}

				ImGui::EndMenu();
			}

			
			if (ImGui::BeginMenu("Windows"))
			{
				//ImGui::MenuItem("PE Infos (PE Bear inspired)", "", &MainConfig.windowToggles.showFileInformations);

				g_ModuleManager.RenderToolWindow();

				ImGui::ItemSize(ImVec2(0, 0));

				if (ImGui::MenuItem("Clear Console")) g_MainLogger.ClearConsole();

				ImGui::EndMenu();
			}
			

			ImGui::EndMainMenuBar();
		}
		if (currentParsedFile.hasAFile)
		{
			g_ModuleManager.RenderWindows();

		}
		// down bar
		{
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(31, 33, 38).Value);

			ImVec2 windowLogSize = {displaySize.x, 200};


			if (ImGui::Begin("##Log", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
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
}