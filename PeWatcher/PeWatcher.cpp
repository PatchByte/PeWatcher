#include <iostream>
#include "PeFile.hpp"
#include "gui/Renderer.h"

static PeReaderParser currentParsedFile = PeReaderParser();

int main(int argc, char* argv[])
{

	if (argc < 2)
	{
		printf("./PeWatcher [exe]\n");
		return -1;
	}

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
						currentParsedFile.Initialize(filename);
					}
				}

				if (ImGui::MenuItem("Close", "", false, currentParsedFile.hasAFile))
				{
					currentParsedFile.Release();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (ImGui::Begin("Loader", 0))
		{

		}
		ImGui::End();

		// down bar
		{
			static ImVec2 windowLogSize = {150, 200};
			static bool downbarUp = false;
			
			if (ImGui::Begin("Log", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
			{
				
			}

			downbarUp = !ImGui::IsWindowCollapsed();

			if (downbarUp == false)
			{
				auto inVSize = displaySize; inVSize.x = 0;
				inVSize.y -= ImGui::GetStyle().DisplayWindowPadding.y;
				ImGui::SetWindowPos(inVSize);
			}
			else
			{
				auto inVSize = displaySize; inVSize.x = 0;
				inVSize.y -= windowLogSize.y;
				ImGui::SetWindowPos(inVSize);
			}

			{
				auto sizeBefore = windowLogSize;
				ImGui::SetWindowSize(sizeBefore);
			}

			ImGui::End();
		}

		g_MainRender.EndRender();
	}
	g_MainRender.UnInitialize();

	
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