#include <iostream>
#include "PeFile.hpp"

int main(int argc, char* argv[])
{

	if (argc < 2)
	{
		printf("./PeWatcher [exe]\n");
		return -1;
	}

	
	PeReaderParser reader = PeReaderParser();
	reader.Initialize(argv[1]);
	reader.Parse();
	printf("State Readed: %s\n", argv[1]);
	printf("Sections: %i\n", reader.GetParser()->GetSectionNumbers());
	for (int i = 0; i < reader.GetParser()->GetSectionNumbers(); i++)
	{
		auto currentSection = reader.GetParser()->GetSectionHeaderByIndex(i);
		printf("-- %s\n", currentSection->Name);
	}

	printf("State Mapping!\n");
	PeMapper_Normal normalMapper = PeMapper_Normal();
	normalMapper.Initialize(reader.GetParser());

	normalMapper.MapPeFile();

	printf("-- Mapped memory address: %p\n", normalMapper.GetBaseAddr());

	while (true)
	{
		if (GetAsyncKeyState(VK_INSERT) & 1) { break; }
	}

	printf("Running\n");
	auto pEntry = normalMapper.GetBaseAddr() + normalMapper.GetNtHeaders()->OptionalHeader.AddressOfEntryPoint;

	

	typedef void(*EntryFuncDef)();

	((EntryFuncDef)pEntry)();

	normalMapper.Release();
	reader.Release();
}