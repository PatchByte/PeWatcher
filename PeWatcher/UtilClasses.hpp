#pragma once
#include <Windows.h>

// rip. i sin c gods. please forgive me
#include <map>
#include <vector>

struct Import
{
	HMODULE from;
	char* name;

	PVOID dest;
	PVOID trampDest;
};

struct Imports
{
	std::vector<Import*> imports;
	HMODULE lib;

	void Release()
	{
		for (auto& x : imports)
		{
			free(x);
		}
	}
};

class ImportHolder
{
private:
	std::map<HMODULE, Imports*> allImports;
	std::map<HMODULE, bool> loadedLib;
public:
	Imports* ImportLib(char* name)
	{
		
		HMODULE module = GetModuleHandleA(name) != 0 ? GetModuleHandleA(name) : LoadLibraryA(name);
		loadedLib.emplace(module, true);

		Imports* importHolding = (Imports*)malloc(sizeof(Imports));
		allImports.emplace(module, importHolding);

		importHolding->lib = module;

		return importHolding;
	}

	Import* Resolve(HMODULE mod, char* methodName)
	{

		auto proc = (PVOID)GetProcAddress(mod, (LPCSTR)methodName);

		if (proc == 0) return 0;

		auto importHolding = allImports[mod];
		Import *cimport = (Import*)malloc(sizeof(Import));
		cimport->dest = proc;
		cimport->from = mod;
		cimport->name = methodName;

		// that's gonna be fun later
		cimport->trampDest = nullptr;

		//importHolding->imports.push_back(cimport);

		return cimport;
	}
	
	auto& GetImportHolder(HMODULE mod)
	{
		return this->allImports[mod];
	}

	bool HasLoadedLib(char* name) { return GetModuleHandleA(name) != 0; }

	void Release()
	{
		for (auto &x : allImports)
		{
			x.second->Release();
		}

		allImports.clear();
		loadedLib.clear();
	}
};

// config classes

class Config
{
public:
	bool hasMapped;

};

inline Config MainConfig;

// Machine Name Util

const char* GetMachineTypeString(WORD machine)
{
	const char* type = NULL;

	switch (machine)
	{
	case IMAGE_FILE_MACHINE_I386: { type = "Intel 386"; break;  }
	case IMAGE_FILE_MACHINE_IA64: { type = "Intel 64"; break;  }
	case IMAGE_FILE_MACHINE_AMD64: { type = "AMD 64"; break;  }
	default:
		break;
	}

	return type;
}

// handy macro
#define DISP_OFFSET(stru, mem,nam, form, val) ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("%X", (uint32_t)offsetof(stru, mem)); if(ImGui::IsItemHovered()) { ImGui::BeginTooltip();ImGui::Text("%s", #mem);ImGui::EndTooltip(); } ImGui::TableNextColumn(); ImGui::Text(nam); ImGui::TableNextColumn(); ImGui::Text(form, val->mem); 
