#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdint>
#include <vector>

__declspec(noinline) auto locateTextSections(const std::vector<char>& bytes) {
	auto result = std::vector<std::pair<const void*, size_t>>{};
	if (bytes.data() && !bytes.empty()) {
		auto base = bytes.data();
		auto dosImage = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
		if (dosImage->e_magic == 0x5A'4D) {
			int ntOffset = dosImage->e_lfanew;
			if (ntOffset > 0 && ntOffset < bytes.size()) {
				auto ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS32*>(base + ntOffset);
				if (ntHeaders->Signature == 0x00'00'45'50) {
					auto sections = reinterpret_cast<const IMAGE_SECTION_HEADER*>((uintptr_t)ntHeaders + sizeof(ntHeaders->Signature) + sizeof(ntHeaders->FileHeader) + ntHeaders->FileHeader.SizeOfOptionalHeader);
					auto numSections = ntHeaders->FileHeader.NumberOfSections;
					for (int i = 0; i < numSections; ++i) {
						auto& section = sections[i];
						auto name64 = *reinterpret_cast<const uint64_t*>(&section.Name);
						if (name64 == 0x00'00'00'74'78'65'74'2Eull) {
							auto sectionStart = reinterpret_cast<const void*>(base + section.PointerToRawData);
							auto sectionSize = static_cast<size_t>(section.SizeOfRawData);
							result.push_back({ sectionStart, sectionSize });
						}
					}
				}
			}
		}
		if (result.empty()) result.push_back({ bytes.data(), bytes.size() });
	}
	return result;
}