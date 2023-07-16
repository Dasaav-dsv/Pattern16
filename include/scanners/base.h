#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <cstdint>
#include <intrin.h>
#include <immintrin.h>

#include "../util.h"
#include "../pfreq.h"

#ifdef PATTERN16_64BIT
#include "x64/x64.h"
#include "x64/SSE.h"
#include "x64/AVX.h"
#else 
#error 32-bit compilation is not currently supported by Pattern16
#endif

namespace Pattern16 {
	namespace Impl {
		PATTERN16_NO_INLINE const void* scanRegion(const void* regionStart, const void* regionEnd, const SplitSignatureU8& signature) {
			auto length = signature.first.size();
			auto cur = reinterpret_cast<const uint8_t*>(regionStart) - 1;
			auto end = reinterpret_cast<const uint8_t*>(regionEnd) - signature.first.size();
			while PATTERN16_LIKELY (++cur < end) {
				auto length_ = length;
				while (length_--) {
					if (!length_) return (const void*)cur;
					auto potential_match = cur[length_];
					potential_match ^= signature.first[length_];
					if (potential_match & signature.second[length_]) break;
				}
			};
			return nullptr;
		}

		alignas(64) inline constexpr const int8_t hexLookup[128] = {
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
			-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,91,-1,93,-1,-1,
			-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		};

		PATTERN16_NO_INLINE SplitSignatureU8 processSignatureString(const std::string& signature) {
			Impl::SplitSignatureU8 splitSignatureBytes{};
			auto& [sig, mask] = splitSignatureBytes;
			std::string byteStr;
			auto bit = -1;
			auto counter = 0;
			for (auto chr : signature) {
				if (chr >= 0) {
					if (chr == ' ');
					else if (chr == '[') bit = 0;
					else if (chr == ']') bit = -1;
					else {
						auto val = hexLookup[chr];
						if (bit < 0) {
							if ((++counter &= 1)) {
								sig.push_back(0);
								mask.push_back(0);
							}
							if (val >= 0) {
								sig.back() |= val << (counter << 2);
								mask.back() |= 0xF << (counter << 2);
							}
						}
						else {
							if (--bit < 0) {
								sig.push_back(0);
								mask.push_back(0);
								counter = 0;
								bit &= 7;
							}
							if (val >= 0) {
								sig.back() |= (val & 1) << bit;
								mask.back() |= 1 << bit;
							}
						}
					}
				}
			}
			return splitSignatureBytes;
		}

		template <typename T>
		PATTERN16_NO_INLINE auto processSignatureBytes(SplitSignatureU8 signature) {
			auto& signatureBytes = signature.first;
			auto& maskBytes = signature.second;
			auto new_size = alignUp<alignof(T)>(signatureBytes.size()) / sizeof(T);
			signatureBytes.resize(new_size * sizeof(T), 0);
			maskBytes.resize(new_size * sizeof(T), 0);
			SplitSignature<T> processed{ std::vector<T>(new_size), std::vector<T>(new_size) };
			std::memcpy((void*)processed.first.data(), (void*)signatureBytes.data(), signatureBytes.size());
			std::memcpy((void*)processed.second.data(), (void*)maskBytes.data(), maskBytes.size());
			return processed;
		}

		template <BMI_VERSION version>
		PATTERN16_NO_INLINE auto getSigStartPos(const SplitSignatureU8& signature, const Frequencies16& cache) {
			std::vector<uint16_t> frequencies(signature.first.size() - 1);
			int offset = 0;
			for (auto& fq : frequencies) {
				auto index = *reinterpret_cast<const uint16_t*>(signature.first.data() + offset);
				index = _pext_u32_BMI<version>(index, ~0b0001'0000'0000'0011);
				fq = cache[index];
				fq = *reinterpret_cast<const uint16_t*>(signature.second.data() + offset) != 0xFFFF ? 0xFFFF : fq;
				++offset;
			}
			return std::distance(frequencies.begin(), std::min_element(frequencies.begin(), frequencies.end()));
		}

		template <typename T, SSE_VERSION version = SSE4_1>
		PATTERN16_NO_INLINE const void* scanT(const void* regionStart, size_t regionSize, SplitSignatureU8& signature, const Frequencies16& frequencies) {
			auto alignedStart = alignUpCacheline(regionStart);
			if (auto address = scanRegion(regionStart, alignedStart, signature)) return address;
			size_t sigStartPos;
			std::array<int, 4> cpuInfo;
			{
				PATTERN16_CPUID_LEAF7(cpuInfo);
				if (PATTERN16_FEATURE_TEST(cpuInfo, PATTERN16_FEATURE_BMI2)) sigStartPos = getSigStartPos<BMI2>(signature, frequencies);
				else if (PATTERN16_FEATURE_TEST(cpuInfo, PATTERN16_FEATURE_BMI1)) sigStartPos = getSigStartPos<BMI1>(signature, frequencies);
				else sigStartPos = getSigStartPos<BMI_NONE>(signature, frequencies);
			}
			auto sig = processSignatureBytes<T>(signature);
			if constexpr (version == SSE4_1) return scanRegion(alignedStart, (const void*)((uintptr_t)regionStart + regionSize), sigStartPos, 0, sig, sig.first.size());
			else return scanRegion<version>(alignedStart, (const void*)((uintptr_t)regionStart + regionSize), sigStartPos, 0, sig, sig.first.size());
		}

		PATTERN16_NO_INLINE const void* scan(const void* regionStart, size_t regionSize, SplitSignatureU8& signature, const Frequencies16& frequencies) {
			std::array<int, 4> cpuInfo;
			PATTERN16_CPUID_LEAF7(cpuInfo);
			if (PATTERN16_FEATURE_TEST(cpuInfo, PATTERN16_FEATURE_AVX2)) {
				return scanT<__m256i>(regionStart, regionSize, signature, frequencies);
			}
			PATTERN16_CPUID_LEAF1(cpuInfo);
			if (PATTERN16_FEATURE_TEST(cpuInfo, PATTERN16_FEATURE_SSE4_1)) {
				return scanT<__m128i, SSE4_1>(regionStart, regionSize, signature, frequencies);
			}
			else if (PATTERN16_FEATURE_TEST(cpuInfo, PATTERN16_FEATURE_SSE2)) {
				return scanT<__m128i, SSE2>(regionStart, regionSize, signature, frequencies);
			}
			else {
				return scanT<uint64_t>(regionStart, regionSize, signature, frequencies);
			}
		}
	}
}