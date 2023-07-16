#pragma once

#include <array>
#include <vector>
#include <string>
#include <memory>
#include <fstream>

#include "../../include/Pattern16.h"

auto fileToVec(std::ifstream& file, std::vector<char>& out) {
	file.seekg(0, std::ios::end);
	auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	out.resize(size);
	file.read(reinterpret_cast<char*>(out.data()), size);
	return out;
}

void cacheToFile(std::ofstream& file, const Pattern16::Impl::CacheSerialized<0x2000>& in) {
	auto cur = 0ull;
	auto raw = in.data();
	auto lines = (in.size() / 16) - 1;
	file << std::uppercase << std::hex;
	if (lines > 0) {
		for (int i = 0; i < lines; ++i) {
			for (int j = 0; j < 15; ++j) {
				auto val = static_cast<int>(raw[cur + j]);
				file << (val > 0x0F ? "0x" : "0x0");
				file << val << ", ";
			}
			auto val = static_cast<int>(raw[cur + 15]);
			file << (val > 0x0F ? "0x" : "0x0");
			file << val << ",\n";
			cur += 16;
		}
	}
	if (lines >= 0) {
		for (int i = 0; i < 15; ++i) {
			auto val = static_cast<int>(raw[cur + i]);
			file << (val > 0x0F ? "0x" : "0x0");
			file << val << ", ";
		}
		auto val = static_cast<int>(raw[cur + 15]);
		file << (val > 0x0F ? "0x" : "0x0");
		file << val << ",";
	}
	file << std::dec;
}

void getRegionFrequencies(const std::pair<const void*, size_t>& region, Pattern16::Impl::Frequencies& frequencies) {
	const auto& [start, size] = region;
	Pattern16::Impl::getFrequencies16(start, (void*)((uintptr_t)start + size), frequencies);
}

auto serializeFrequencies(const Pattern16::Impl::Frequencies& frequencies) {
	auto out = std::make_unique<Pattern16::Impl::CacheSerialized<0x2000>>();
	std::array<int, 4> cpuInfo;
	{
		PATTERN16_CPUID_LEAF7(cpuInfo);
		if (PATTERN16_FEATURE_TEST(cpuInfo, PATTERN16_FEATURE_BMI2)) Pattern16::Impl::makeFrequencyCache12_t<Pattern16::Impl::BMI2>(frequencies, *out);
		else if (PATTERN16_FEATURE_TEST(cpuInfo, PATTERN16_FEATURE_BMI1)) Pattern16::Impl::makeFrequencyCache12_t<Pattern16::Impl::BMI1>(frequencies, *out);
		else Pattern16::Impl::makeFrequencyCache12_t<Pattern16::Impl::BMI_NONE>(frequencies, *out);
	}
	return out;
}

void cacheFrequencies(std::string name, const Pattern16::Impl::CacheSerialized<0x2000>& frequencies) {
	name += ".freqcache.txt";
	std::ofstream file{ name };
	cacheToFile(file, frequencies);
}

auto processCL(int argc, char* argv[]) {
	auto result = std::vector<std::pair<std::string, std::vector<char>>>{};
	if (argc > 1) {
		for (int i = 1; i < argc; ++i) {
			if (std::ifstream file{ argv[i], std::ios::in | std::ios::binary }) {
				std::vector<char> fileBytes{};
				result.push_back({ argv[i], fileToVec(file, fileBytes) });
			}
		}
	}
	return result;
}