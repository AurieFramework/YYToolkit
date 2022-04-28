#include "PEParser.hpp"
#include <fstream>

DWORD Utils::RVA_To_Offset(PIMAGE_NT_HEADERS pNTHeader, DWORD dwRVA)
{
    PIMAGE_SECTION_HEADER pHeaders = IMAGE_FIRST_SECTION(pNTHeader);

    // Loop over all the sections of the file
    for (int n = 0; n < pNTHeader->FileHeader.NumberOfSections; n++)
    {
        // ... to check if the RVA points to within that section
        // the section begins at pHeaders[n].VirtualAddress and ends at pHeaders[n].VirtualAddress + pHeaders[n].SizeOfRawData
        if (dwRVA >= pHeaders[n].VirtualAddress && dwRVA < (pHeaders[n].VirtualAddress + pHeaders[n].SizeOfRawData))
        {
            // The RVA points into this section, so return the offset inside the section's data.
            return (dwRVA - pHeaders[n].VirtualAddress) + pHeaders[n].PointerToRawData;
        }
    }

    return 0;
}

bool Utils::DoesPEExportRoutine(const wchar_t* FilePath, const char* RoutineName)
{
    // Open the file
    std::ifstream fFileStream(FilePath, std::ios::binary | std::ios::ate);

    // If the file failed to open
    if (!fFileStream.is_open())
        return false;

    std::streampos spFileSize = fFileStream.tellg();

    // If the file is below 4kB
    if (spFileSize <= 4096)
        return false;

    BYTE* pSource = new BYTE[static_cast<UINT>(spFileSize)];

    if (!pSource)
        return false;

    // Go to the beginning
    fFileStream.seekg(0, std::ios::beg);

    // Read the whole file and copy the contents into pSource
    fFileStream.read(reinterpret_cast<char*>(pSource), spFileSize);

    // Cast the beginning of the PE file to a DOS Header
    PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(pSource);

    // If a cosmic ray flipped some bits
    if (!pDosHeader)
    {
        if (pSource)
            delete[] pSource;

        return false;
    }

    //MZ backwards because of endianness reasons
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        delete[] pSource;
        return false;
    }

    // Follow DOS->pPEHeader and cast it to the right type
    PIMAGE_NT_HEADERS pNTHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(pSource + pDosHeader->e_lfanew);

    // If there's no NT Headers
    if (!pNTHeader)
    {
        delete[] pSource;
        return false;
    }

    // Check for PE sig
    if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
    {
        delete[] pSource;
        return false;
    }

    // If there's no Export Table
    if (pNTHeader->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_EXPORT)
    {
        delete[] pSource;
        return false;
    }

    // Get the first header pointer
    DWORD dwRVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

    // If the RVA is not found (huh?)
    if (!(dwRVA = RVA_To_Offset(pNTHeader, dwRVA)))
    {
        delete[] pSource;
        return false;
    }

    // Get the pointer to the actual struct through the RVA
    auto pExportDirectory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(pSource + dwRVA);

    // If it was null
    if (!pExportDirectory)
    {
        delete[] pSource;
        return false;
    }

    // Calculate the RVA address of the names array
    if (!(dwRVA = RVA_To_Offset(pNTHeader, pExportDirectory->AddressOfNames)))
    {
        delete[] pSource;
        return false;
    }
    
    // Create the array pointer
    DWORD* pNamesAddress = reinterpret_cast<DWORD*>(pSource + dwRVA);

    for (int i = 0; i < pExportDirectory->NumberOfNames; i++)
    {
        if (!(dwRVA = RVA_To_Offset(pNTHeader, pNamesAddress[i])))
            continue;

        const char* pEntryName = reinterpret_cast<const char*>(pSource + dwRVA);

        if (!strcmp(pEntryName, RoutineName))
        {
            delete[] pSource;
            return true;
        }
    }

    delete[] pSource;
    return false;
}