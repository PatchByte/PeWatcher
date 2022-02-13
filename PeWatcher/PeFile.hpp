#pragma once
#include <Windows.h>
#include <stdio.h>

#include "UtilClasses.hpp"

#if _WIN64
typedef DWORD64 DWM;
#else
typedef DWORD DWM;
#endif

#define RELOC_FLAG32(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#define MUST_NT_HEADER IMAGE_NT_OPTIONAL_HDR64_MAGIC
#define NT_ONLY_STR "64 bit"
#else
#define RELOC_FLAG RELOC_FLAG32
#define MUST_NT_HEADER IMAGE_NT_OPTIONAL_HDR32_MAGIC
#define NT_ONLY_STR "32 bit"
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

	ImportHolder imports;

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
		return (PIMAGE_NT_HEADERS32)((DWM)peFileInMemory + GetDosHeader()->e_lfanew);
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
	bool MapPeFile()
	{
		DWORD oldProtect = 0;

		peFileImageSize = parsedPeFile->GetNtHeaders()->OptionalHeader.SizeOfImage;
		peFileInMemory = VirtualAlloc(0, peFileImageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

#ifdef _DEBUG
		printf("State: Writing Dos Header!\n");
#endif

		{
			memcpy((PIMAGE_DOS_HEADER)peFileInMemory, parsedPeFile->GetDosHeader(), sizeof(IMAGE_DOS_HEADER));
		}

#ifdef _DEBUG
		printf("State: Writing Nt Header!\n");
#endif

		{
			auto ntData = parsedPeFile->GetNtHeaders();
			auto ntOffset = (DWM)peFileInMemory;
			ntOffset += parsedPeFile->GetDosHeader()->e_lfanew;

			memcpy((PVOID)ntOffset, ntData, sizeof(*ntData));

			
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

		{
			auto optionalHeader = &this->GetNtHeaders()->OptionalHeader;

			{
				BYTE* LocationDelta = (BYTE*)(this->GetBaseAddr() - optionalHeader->ImageBase);
				if (LocationDelta)
				{
					if (!optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
						return false;

					auto* pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(this->GetBaseAddr() + optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
					while (pRelocData->VirtualAddress)
					{
						UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
						WORD* pRelativeInfo = reinterpret_cast<WORD*>(pRelocData + 1);

						for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo)
						{
							if (RELOC_FLAG(*pRelativeInfo))
							{
								UINT_PTR* pPatch = reinterpret_cast<UINT_PTR*>(this->GetBaseAddr() + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
								*pPatch += reinterpret_cast<UINT_PTR>(LocationDelta);
							}
						}
						pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(pRelocData) + pRelocData->SizeOfBlock);
					}
				}
			}
			{
				if (optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
				{
					PIMAGE_IMPORT_DESCRIPTOR currentLib = (PIMAGE_IMPORT_DESCRIPTOR)(this->GetBaseAddr() + optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
					while (currentLib->Name)
					{
						char* importName = (char*)(this->GetBaseAddr() + currentLib->Name);

						printf("Importing Lib: %s\n", importName);
						auto modResolved = imports.ImportLib(importName);

						{

							ULONG_PTR* pThunkRef = (ULONG_PTR*)(this->GetBaseAddr() + currentLib->OriginalFirstThunk);
							ULONG_PTR* pFuncRef = (ULONG_PTR*)(this->GetBaseAddr() + currentLib->FirstThunk);

							if (!pThunkRef)
								pThunkRef = pFuncRef;

							for (; *pThunkRef; ++pThunkRef, ++pFuncRef)
							{
								if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef))
								{
									*pFuncRef = (ULONG_PTR)imports.Resolve(modResolved->lib, reinterpret_cast<char*>(*pThunkRef & 0xFFFF))->dest;
									printf("\t-- Import Oridinal: %s > %p\n", reinterpret_cast<char*>(*pThunkRef & 0xFFFF), *pFuncRef);
								}
								else
								{
									auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(this->GetBaseAddr() + (*pThunkRef));

									*pFuncRef = (ULONG_PTR)imports.Resolve(modResolved->lib, pImport->Name)->dest;
									printf("\t-- Import Name: %s > %p\n", pImport->Name, *pFuncRef);
								}


							}

						}
						

						currentLib = &currentLib[1];
					}
				}
			}

		}

		
		this->GetNtHeaders()->OptionalHeader.ImageBase = (DWM)peFileInMemory;

		return true;
	}

	/*
		Just deallocates it -_-
	*/
	void Release()
	{
		imports.Release();
		VirtualFree(peFileInMemory, 0, MEM_RELEASE);
		printf("Released!\n");
	}
};

class PeReaderParser
{
private:
	PeFileParser parser;
	PeMapper_Normal normalMapper;
	void* fileContent;
	FILE* fileStream;
	char* fileName;
	size_t fileSize;
	
public:
	bool hasAFile = false;
	char* absName = 0;


	PeReaderParser() = default;
	~PeReaderParser() = default;

	void Initialize(char* path)
	{
		hasAFile = true;
		fileName = path;

		absName = strrchr(path, '\\');
		if (absName == 0)
		{
			absName = strrchr(path, '/');
		}
		if (absName != 0) absName++;

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

	bool Parse()
	{
		parser = PeFileParser();
		parser.Initialize(fileContent);
		

		if (parser.GetNtHeaders()->OptionalHeader.Magic != MUST_NT_HEADER) { Release(); return false; }

		return true;
	}

	PeFileParser* GetParser()
	{
		return &parser;
	}

	PeMapper_Normal* GetNormalMapper()
	{
		return &normalMapper;
	}

	void Release()
	{
		VirtualFree(fileContent, 0, MEM_RELEASE);
		hasAFile = false;
	}
};

inline PeReaderParser currentParsedFile = PeReaderParser();