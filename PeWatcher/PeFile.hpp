#include <Windows.h>

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
};