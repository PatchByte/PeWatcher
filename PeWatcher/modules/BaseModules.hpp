#pragma once
#include <iostream>
#include <Windows.h>
#include <vector>

#include <imgui.h>
#include "../PeFile.hpp"

#define REGISTER_MODULE(cClass) inline ModuleRegister<cClass> REG##cClass = ModuleRegister<cClass>();

class BaseModuleGui
{
public:
public:
	virtual void RenderWindow() {};
	virtual const char* GetWindowName() { return NULL; };
	virtual void Initialize() {};
	virtual void UnInitialize() {};
public:
	bool m_opend;
};

class ModuleManager
{
public:

	template<typename TClass>
	void Registered()
	{
		TClass* cls = new TClass();
		cls->Initialize();
		m_registeredModules.push_back(cls);
	}

	void RenderToolWindow()
	{
		for (auto& x : m_registeredModules)
		{
			ImGui::MenuItem(x->GetWindowName(), "", &x->m_opend);
		}
	}

	void RenderWindows()
	{
		for (auto& x : m_registeredModules)
		{
			if (x->m_opend)
			{
				ImGui::Begin(x->GetWindowName());
				x->RenderWindow();
				ImGui::End();
			}
		}
	}

	void Release()
	{
		for (auto& x : m_registeredModules)
		{
			x->UnInitialize();
			delete x;
		}

		m_registeredModules.clear();
	}
private:
	std::vector<BaseModuleGui*> m_registeredModules;
};

inline ModuleManager g_ModuleManager = ModuleManager();

template<typename RClass>
class ModuleRegister
{
public:
	ModuleRegister()
	{
		g_ModuleManager.Registered<RClass>();
	}
};