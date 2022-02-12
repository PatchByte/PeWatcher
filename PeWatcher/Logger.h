#pragma once
#include <Windows.h>
#include <iostream>
#include <cstdarg>

#include <vector>

#include "imgui.h"

class Logger
{
public:

	void Release()
	{
		for (auto& x : m_messages)
		{
			free(x);
		}
		m_messages.clear();
	}

	void LogInfo(const char* format, ...)
	{
		va_list vaList;

		va_start(vaList, format);

		AddMessageToMessages("Info", format, vaList);

		va_end(vaList);
	}

	void LogError(const char* format, ...)
	{
		va_list vaList;

		va_start(vaList, format);

		AddMessageToMessages("Error", format, vaList);

		va_end(vaList);
	}

public:

	void Render(ImFont* renderFont)
	{
		ImGui::PushFont(renderFont);
		for (auto& msg : this->m_messages)
		{
			ImGui::Text("%s", msg);
			
		}
		ImGui::PopFont();
	}

private:

	void AddMessageToMessages(const char* type, const char* format, va_list list)
	{
		
		if (m_messages.size() > GetMaxMessages()) { free(*m_messages.end());  m_messages.pop_back(); }
		
		auto time = std::time(nullptr);
		struct tm _tm = {};
		localtime_s(&_tm, &time);
		char prefix[64] = {};
		std::snprintf(prefix, sizeof(prefix) - 1, "[%02d:%02d:%02d] [%s] ", _tm.tm_hour, _tm.tm_min, _tm.tm_sec, type);

		size_t formatBytes = vsnprintf(0, 0, format, list) + 1;
		size_t neededBytes = formatBytes + strlen(prefix) + 1;
		void* allocatedBytes = malloc(neededBytes);
		memset(allocatedBytes, 0, neededBytes);

		vsnprintf((char*)(PVOID)(allocatedBytes) + strlen(prefix), formatBytes, format, list);
		memcpy(allocatedBytes, prefix, strlen(prefix));

		m_messages.push_back((char*)allocatedBytes);
	}

	void RawMessage(const char* msg)
	{
		if (m_messages.size() > GetMaxMessages()) { free(*m_messages.end());  m_messages.pop_back(); }

		auto len = strlen(msg);
		void* allocatedBytes = malloc(len+1);
		memcpy(allocatedBytes, msg, len+1);
		m_messages.push_back((char*)allocatedBytes);
	}

public:

	inline int GetMaxMessages() { return 200; }

	auto& GetMessages() { return m_messages; }

	void ClearConsole()
	{
		for (auto& x : m_messages)
		{
			free(x);
		}
		m_messages.clear();

		RawMessage("[Action] => Console cleared");
	}

private:
	std::vector<char*> m_messages;
};

inline Logger g_MainLogger;

#ifndef LogInfo
#define LogInfo g_MainLogger.LogInfo
#define LogError g_MainLogger.LogError
#endif