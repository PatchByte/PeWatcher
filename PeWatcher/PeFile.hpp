#pragma once
#include <Windows.h>
#include <stdio.h>

#if _WIN64
typedef DWORD64 DWM;
#else
typedef DWORD DWM;
#endif

class PeFileParser
{
private:
	void* bytePtr;
public:
	PeFileParser() = default;
	~PeFileParser() = default;

	inline void Initialize(void* x)
	{
		bytePtr = x;
	}

	inline PIMAGE_DOS_HEADER GetDosHeader()
	{
		return (PIMAGE_DOS_HEADER)bytePtr;
	}

#if !_WIN64
	inline PIMAGE_NT_HEADERS32 GetNtHeaders()
	{
		return (PIMAGE_NT_HEADERS32)((DWM)bytePtr + GetDosHeader()->e_lfanew);
	}
#else
	inline PIMAGE_NT_HEADERS64 GetNtHeaders()
	{
		return (PIMAGE_NT_HEADERS64)((DWM)bytePtr + GetDosHeader()->e_lfanew);
	}
#endif

	inline PIMAGE_SECTION_HEADER GetSectionHeaderByIndex(int idx)
	{
		PIMAGE_SECTION_HEADER firstSection = IMAGE_FIRST_SECTION(GetNtHeaders());

		return (PIMAGE_SECTION_HEADER)((DWM)firstSection + (idx * sizeof(IMAGE_SECTION_HEADER)));
	}

	inline DWM GetSectionNumbers()
	{
		return GetNtHeaders()->FileHeader.NumberOfSections;
	}


	inline DWM GetBaseAddr()
	{
		return (DWM)bytePtr;
	}
}; 


class PeMapper_Normal
{
private:
	PeFileParser* parsedPeFile;
	void* peFileInMemory;
	size_t peFileImageSize;
public:

	PeMapper_Normal() = default;
	~PeMapper_Normal() = default;

	void Initialize(PeFileParser* pParser) { parsedPeFile = pParser; }

	inline PIMAGE_DOS_HEADER GetDosHeader()
	{
		return (PIMAGE_DOS_HEADER)peFileInMemory;
	}

#if !_WIN64
	inline PIMAGE_NT_HEADERS32 GetNtHeaders()
	{
		return (PIMAGE_NT_HEADERS32)((DWM)bytePtr + GetDosHeader()->e_lfanew);
	}
#else
	inline PIMAGE_NT_HEADERS64 GetNtHeaders()
	{
		return (PIMAGE_NT_HEADERS64)((DWM)peFileInMemory + GetDosHeader()->e_lfanew);
	}
#endif

	inline PIMAGE_SECTION_HEADER GetSectionHeaderByIndex(int idx)
	{
		PIMAGE_SECTION_HEADER firstSection = IMAGE_FIRST_SECTION(GetNtHeaders());

		return (PIMAGE_SECTION_HEADER)((DWM)firstSection + (idx * sizeof(IMAGE_SECTION_HEADER)));
	}

	inline DWM GetSectionNumbers()
	{
		return GetNtHeaders()->FileHeader.NumberOfSections;
	}

	inline DWM GetBaseAddr()
	{
		return (DWM)peFileInMemory;
	}
	
	/*
		Only maps the pe file into memory *!* it doesnt execute's it!
	*/
	void MapPeFile()
	{
		DWORD oldProtect = 0;

		peFileImageSize = parsedPeFile->GetNtHeaders()->OptionalHeader.SizeOfImage;
		peFileInMemory = VirtualAlloc(0, peFileImageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

#ifdef _DEBUG
		printf("State: Writing Dos Header!\n");
#endif

		{
			memcpy(peFileInMemory, parsedPeFile->GetDosHeader(), sizeof(IMAGE_DOS_HEADER));
		}

#ifdef _DEBUG
		printf("State: Writing Nt Header!\n");
#endif

		{
			auto ntData = parsedPeFile->GetNtHeaders();
			auto ntOffset = (DWM)peFileInMemory;
			ntOffset += parsedPeFile->GetDosHeader()->e_lfanew;

			memcpy((PVOID)ntOffset, ntData, sizeof(*ntData));

			this->GetNtHeaders()->OptionalHeader.ImageBase = (DWM)peFileInMemory;
		}

#ifdef _DEBUG
		printf("State: Writing Section's!\n");
#endif

		{
			auto newNtHeader = this->GetNtHeaders();
			auto oldNtHeader = parsedPeFile->GetNtHeaders();

			auto newSectionBegin = IMAGE_FIRST_SECTION(newNtHeader);
			auto oldSectionBegin = IMAGE_FIRST_SECTION(oldNtHeader);

			for (int i = 0; i < oldNtHeader->FileHeader.NumberOfSections; i++)
			{
				auto newCurrentSection = (PIMAGE_SECTION_HEADER)((DWM)newSectionBegin + (i * sizeof(IMAGE_SECTION_HEADER)));
				auto oldCurrentSection = (PIMAGE_SECTION_HEADER)((DWM)oldSectionBegin + (i * sizeof(IMAGE_SECTION_HEADER)));

				memcpy(newCurrentSection, oldCurrentSection, sizeof(IMAGE_SECTION_HEADER));

#ifdef _DEBUG
				printf("State: Mapping Section %.*s! Memory Address: %p\n", 8, newCurrentSection->Name, this->GetBaseAddr() + newCurrentSection->VirtualAddress);
#endif

				if (oldCurrentSection->SizeOfRawData != 0)
				{
					
					memcpy((PVOID)(this->GetBaseAddr() + newCurrentSection->VirtualAddress), (PVOID)(parsedPeFile->GetBaseAddr() + oldCurrentSection->PointerToRawData), oldCurrentSection->SizeOfRawData);
				}

			}
		}

#ifdef _DEBUG
		printf("State: Fixing Imports!\n");
#endif
	}

	/*
		Just deallocates it -_-
	*/
	void Release()
	{
		VirtualFree(peFileInMemory, peFileImageSize, MEM_RELEASE);
	}
};

class PeReaderParser
{
private:
	PeFileParser parser;
	void* fileContent;
	FILE* fileStream;
	char* fileName;
	size_t fileSize;
public:

	PeReaderParser() = default;
	~PeReaderParser() = default;

	void Initialize(char* path)
	{
		fileName = path;
		fopen_s(&fileStream, fileName, "rb");

		{
			fseek(fileStream, 0, SEEK_END);
			fileSize = ftell(fileStream);
			fseek(fileStream, 0, SEEK_SET);
		}

		fileContent = VirtualAlloc(0, fileSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		fread(fileContent, fileSize, 1, fileStream);

		fclose(fileStream);
		fileStream = 0;
	}

	void Parse()
	{
		parser = PeFileParser();
		parser.Initialize(fileContent);
	}

	PeFileParser* GetParser()
	{
		return &parser;
	}

	void Release()
	{
		VirtualFree(fileContent, fileSize, MEM_RELEASE);
	}
};