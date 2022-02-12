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
		imports.clear();
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
	struct WindowToggles
	{
		bool showFileInformations;
	} windowToggles;

};

inline Config MainConfig;
