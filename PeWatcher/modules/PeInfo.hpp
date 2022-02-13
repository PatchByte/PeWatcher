#include "BaseModules.hpp"

class PeInfoModuleGui : public BaseModuleGui
{
public:
	const char* GetWindowName()
	{
		return "PeInfo (like PeBear)";
	}

	void RenderWindow()
	{
		{
			static short selectedTab = -1;

#define MAKE_SELECTABLE(name, idx, tabName) if(ImGui::Selectable(name, false)) { selectedTab = idx; }

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
				if (ImGui::BeginTable("##dosheader_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_RowBg))
				{

					ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Offset");
					ImGui::TableNextColumn();
					ImGui::Text("Name");
					ImGui::TableNextColumn();
					ImGui::Text("Value");

					auto dHead = currentParsedFile.GetParser()->GetDosHeader();



					DISP_OFFSET(IMAGE_DOS_HEADER, e_magic, "Magic Number", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_cblp, "Bytes on last page of file", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_cp, "Pages in file", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_crlc, "Relocations", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_cparhdr, "Size of header in paragraphs", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_minalloc, "Minimum extra paragraphs needed", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_maxalloc, "Maximum extra paragraphs needed", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_ss, "Initial (relative) SS value", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_sp, "Initial SP value", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_csum, "Checksum", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_ip, "Initial IP value", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_cs, "Initial (relative) CS value", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_lfarlc, "File address of relocation table", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_ovno, "Overlay number", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_res, "Reserved words", "", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_oemid, "OEM identifier (for e_oeminfo)", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_oeminfo, "OEM information; e_oemid specific", "%X", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_res2, "Reserved words", "", dHead);
					DISP_OFFSET(IMAGE_DOS_HEADER, e_lfanew, "File address of new exe header", "%X", dHead);

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("NT Header's"))
			{
				ImGui::BeginTabBar("##ntundertabbar");

				if (ImGui::BeginTabItem("FileHeader"))
				{
					auto fHead = &currentParsedFile.GetParser()->GetNtHeaders()->FileHeader;

					if (ImGui::BeginTable("##dosheader_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_RowBg))
					{

						ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Offset");
						ImGui::TableNextColumn();
						ImGui::Text("Name");
						ImGui::TableNextColumn();
						ImGui::Text("Value");

						{
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("%X", (uint32_t)offsetof(IMAGE_FILE_HEADER, Machine));
							ImGui::TableNextColumn();
							ImGui::Text("Target CPU");
							ImGui::TableNextColumn();
							ImGui::Text("%X", fHead->Machine);
							if (auto typeStr = GetMachineTypeString(fHead->Machine))
							{
								ImGui::SameLine();
								ImGui::PushStyleColor(ImGuiCol_Text, ImColor(9, 132, 227).Value);
								ImGui::Text("(%s)", typeStr);
								ImGui::PopStyleColor();
							}
						}

						DISP_OFFSET(IMAGE_FILE_HEADER, NumberOfSections, "Number of Sections", "%i", fHead);
						{
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("%X", (uint32_t)offsetof(IMAGE_FILE_HEADER, TimeDateStamp));
							ImGui::TableNextColumn();
							ImGui::Text("Build Date");
							ImGui::TableNextColumn();
							ImGui::Text("%X", fHead->TimeDateStamp);

							struct tm _tm = {};
							time_t cvnTime = fHead->TimeDateStamp;
							localtime_s(&_tm, &cvnTime);

							ImGui::SameLine();
							ImGui::PushStyleColor(ImGuiCol_Text, ImColor(9, 132, 227).Value);
							ImGui::Text("(%i:%i:%i)", _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								ImGui::Text("%i.%i. %i (MM:DD YYYY)", _tm.tm_mon + 1, _tm.tm_mday, 1900 + _tm.tm_year);
								ImGui::Text("%i:%i:%i", _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
								ImGui::EndTooltip();
							}
							ImGui::PopStyleColor();
						}
						DISP_OFFSET(IMAGE_FILE_HEADER, PointerToSymbolTable, "Offset to the COFF symbol table", "%X", fHead);
						DISP_OFFSET(IMAGE_FILE_HEADER, NumberOfSymbols, "Number of Symbols in COFF Table", "%X", fHead);
						DISP_OFFSET(IMAGE_FILE_HEADER, SizeOfOptionalHeader, "Size of the Opt. Header", "%X", fHead);
						DISP_OFFSET(IMAGE_FILE_HEADER, Characteristics, "Attributes of the file", "%X", fHead);


						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("OptionalHeader"))
				{
					auto oHead = &currentParsedFile.GetParser()->GetNtHeaders()->OptionalHeader;

					if (ImGui::BeginTable("##dosheader_table", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_RowBg))
					{

						ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Offset");
						ImGui::TableNextColumn();
						ImGui::Text("Name");
						ImGui::TableNextColumn();
						ImGui::Text("Value");
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, Magic, "Magic of the Header", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, MajorLinkerVersion, "Major version of the linker", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, MinorLinkerVersion, "Minor version of the linker", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SizeOfCode, "Total size of all sections", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SizeOfInitializedData, "Combined Size of all initialized Sections", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SizeOfUninitializedData, "Combined Size of all uninitialized Sections", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, AddressOfEntryPoint, "RVA of the Entry Point", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, BaseOfCode, "RVA of the first byte of code", "%X", oHead);
#ifndef _WIN64
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, BaseOfData, "(BaseOfData)", "%X", oHead);
#endif
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, ImageBase, "Preferred Base address in memory", "%p", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SectionAlignment, "The alignment of sections", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, FileAlignment, "The alignment of sections within the PE file", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, MajorOperatingSystemVersion, "Major version number of the required OS", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, MinorOperatingSystemVersion, "Minor version number of the required OS", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, MajorImageVersion, "The major version number of this file", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, MinorImageVersion, "The minor version number of this file", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, MajorSubsystemVersion, "(MajorSubsystemVersion)", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, MinorSubsystemVersion, "(MinorSubsystemVersion)", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, Win32VersionValue, "(Win32VersionValue)", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SizeOfImage, "Toal Size of mapped space in Memory", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SizeOfHeaders, "Combined size of the MS-DOS header", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, CheckSum, "The checksum of the image", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, Subsystem, "User interface type", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, DllCharacteristics, "Characteristics of the DLL", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SizeOfStackReserve, "(SizeOfStackReserve)", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SizeOfStackCommit, "(SizeOfStackCommit)", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SizeOfHeapReserve, "(SizeOfHeapReserve)", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, SizeOfHeapCommit, "(SizeOfHeapCommit)", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, LoaderFlags, "(LoaderFlags)", "%X", oHead);
						DISP_OFFSET(IMAGE_OPTIONAL_HEADER, NumberOfRvaAndSizes, "(NumberOfRvaAndSizes)", "%X", oHead);

						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Section Header's"))
			{

				if (ImGui::BeginTable("##section_table", 9, ImGuiTableFlags_Resizable | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_RowBg))
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
						if (ImGui::IsItemHovered())
						{

						}
						auto cSection = currentParsedFile.GetParser()->GetSectionHeaderByIndex(idx);
						ImGui::TableSetColumnIndex(0);
						ImGui::Selectable("##Section", false, ImGuiSelectableFlags_SpanAllColumns); ImGui::SameLine();
						ImGui::Text("%.*s", 8, cSection->Name);
						ImGui::TableNextColumn();
						ImGui::Text("%X", cSection->PointerToRawData);
						ImGui::TableNextColumn();
						ImGui::Text("%X", cSection->SizeOfRawData);
						ImGui::TableNextColumn();
						ImGui::Text("%X", cSection->VirtualAddress);
						ImGui::TableNextColumn();
						ImGui::Text("%X", cSection->Misc.VirtualSize);
						ImGui::TableNextColumn();
						ImGui::Text("%X", cSection->Characteristics);
						ImGui::TableNextColumn();
						ImGui::Text("%X", cSection->PointerToRelocations);
						ImGui::TableNextColumn();
						ImGui::Text("%X", cSection->NumberOfRelocations);
						ImGui::TableNextColumn();
						ImGui::Text("%X", cSection->NumberOfLinenumbers);

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
	}
};

REGISTER_MODULE(PeInfoModuleGui)