#pragma once

#include <vector>
#include <cstdint>
#include <immintrin.h>

#if defined(__x86_64__) || defined(_M_X64)
#define PATTERN16_64BIT
#else
#define PATTERN16_32BIT
#endif

#if _HAS_CXX20 || __cplusplus >= 202000L
#define PATTERN16_LIKELY(x) (x) [[likely]]
#define PATTERN16_UNLIKELY(x) (x) [[unlikely]]
#elif defined(__GNUC__) || defined(__clang__)
#define PATTERN16_LIKELY(x) (__builtin_expect(bool(x), true))
#define PATTERN16_UNLIKELY(x) (__builtin_expect(bool(x), false))
#else
#define PATTERN16_LIKELY(x) (x)
#define PATTERN16_UNLIKELY(x) (x)
#endif

#if defined(__clang__)
#define PATTERN16_FORCE_INLINE [[gnu::always_inline]] [[gnu::gnu_inline]] extern inline
#elif defined(__GNUC__)
#define PATTERN16_FORCE_INLINE [[gnu::always_inline]] inline
#elif defined(_MSC_VER)
#pragma warning(error: 4714)
#define PATTERN16_FORCE_INLINE __forceinline
#else
#define PATTERN16_FORCE_INLINE inline
#endif

#if defined(_MSC_VER)
#define PATTERN16_NO_INLINE __declspec(noinline)
#else
#define PATTERN16_NO_INLINE __attribute__ ((noinline))
#endif

#if defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#define PATTERN16_CPUID(cpuInfo, leaf) {\
	int EAX, EBX, ECX, EDX; \
	__cpuid(leaf, EAX, EBX, ECX, EDX); \
	cpuInfo = {EAX, EBX, ECX, EDX}; \
}
#else 
#include <intrin.h>
#define PATTERN16_CPUID(cpuInfo, leaf) __cpuid((cpuInfo).data(), leaf)
#endif

#define PATTERN16_CPUID_EAX (0)
#define PATTERN16_CPUID_EBX (1 << 5)
#define PATTERN16_CPUID_ECX (2 << 5)
#define PATTERN16_CPUID_EDX (3 << 5)

#define PATTERN16_CPUID_LEAF1(cpuInfo) PATTERN16_CPUID(cpuInfo, 1)
#define PATTERN16_CPUID_LEAF7(cpuInfo) PATTERN16_CPUID(cpuInfo, 7)

#define PATTERN16_FEATURE(reg, bit) ((reg) | (bit))

// LEAF 1 features:
#define PATTERN16_FEATURE_SSE2 PATTERN16_FEATURE(PATTERN16_CPUID_EDX, 26)
#define PATTERN16_FEATURE_SSE4_1 PATTERN16_FEATURE(PATTERN16_CPUID_ECX, 19)

// LEAF 7 features:
#define PATTERN16_FEATURE_AVX2 PATTERN16_FEATURE(PATTERN16_CPUID_EBX, 5)
#define PATTERN16_FEATURE_BMI1 PATTERN16_FEATURE(PATTERN16_CPUID_EBX, 3)
#define PATTERN16_FEATURE_BMI2 PATTERN16_FEATURE(PATTERN16_CPUID_EBX, 8)

#define PATTERN16_FEATURE_TEST(cpuInfo, feature) (bool(((cpuInfo)[(feature) >> 5] >> ((feature) & 31)) & 1))

namespace Pattern16 {
	namespace Impl {
		template <typename T>
		using SplitSignature = std::pair<std::vector<T>, std::vector<T>>;
		using SplitSignatureU8 = SplitSignature<uint8_t>;

		enum SSE_VERSION {
			SSE2,
			SSE4_1,
		};

		enum BMI_VERSION {
			BMI_NONE,
			BMI1,
			BMI2,
		};

		template <size_t alignment, typename T>
		PATTERN16_FORCE_INLINE constexpr auto alignUp(T value) {
			auto out = (std::size_t)value;
			out = out + (-out & (alignment - 1));
			return out;
		}

		template <typename T>
		PATTERN16_FORCE_INLINE constexpr auto alignUpAddress(T address) {
			return reinterpret_cast<T>(alignUp<alignof(T)>(address));
		}

		template <typename T>
		PATTERN16_FORCE_INLINE constexpr auto alignUpCacheline(T address) {
			return reinterpret_cast<T>(alignUp<64>(address));
		}

		PATTERN16_FORCE_INLINE auto concatBytes(uint8_t first, uint8_t second) {
			return static_cast<uint16_t>((static_cast<uint16_t>(second) << 8) | first);
		}

		PATTERN16_FORCE_INLINE auto broadcastMask64(uint8_t first, uint8_t second) {
			return 0x0001'0001'0001'0001ull * concatBytes(first, second);
		}

		PATTERN16_FORCE_INLINE auto broadcastMask128(uint8_t first, uint8_t second) {
			return _mm_set1_epi16(concatBytes(first, second));
		}

		PATTERN16_FORCE_INLINE auto broadcastMask256(uint8_t first, uint8_t second) {
			return _mm256_set1_epi16(concatBytes(first, second));
		}

		template <SSE_VERSION version>
		PATTERN16_FORCE_INLINE bool _mm_testz_SSE(__m128i _A, __m128i _B) {
			if constexpr (version == SSE2) return static_cast<int16_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_and_si128(_A, _B), _mm_setzero_si128()))) == -1;
			else return _mm_testz_si128(_A, _B);
		}

		PATTERN16_NO_INLINE auto _pext_u32_BMI_NONE(uint32_t a, uint32_t mask) {
			auto result = 0u;
			auto nmask = ~mask;
			while (mask) {
				auto start = _lzcnt_u32(mask);
				auto bitmask = ~0u >> ~start;
				if (nmask &= bitmask) {
					auto offset = _lzcnt_u32(nmask);
					auto len = start - offset;
					auto bitmask2 = bitmask >> len;
					bitmask -= bitmask2;
					mask &= bitmask2;
					result <<= len;
					result |= (a & bitmask) >> ++offset;
				}
				else {
					++start;
					bitmask -= bitmask >> start;
					result <<= start;
					result |= a & bitmask;
					break;
				}
			}
			return result;
		}

		PATTERN16_NO_INLINE auto _pext_u32_BMI1(uint32_t a, uint32_t mask) {
			auto result = 0u;
			auto nmask = ~mask;
			while (mask) {
				auto start = _lzcnt_u32(mask);
				auto bitmask = ~0u >> start;
				if (nmask &= bitmask) {
					auto offset = _lzcnt_u32(nmask);
					auto len = offset - start;
					auto bitmask2 = bitmask >> len;
					bitmask -= bitmask2;
					mask &= bitmask2;
					result <<= len;
					result |= (a & bitmask) >> -offset;
				}
				else {
					bitmask -= bitmask >> -start;
					result <<= -start;
					result |= a & bitmask;
					break;
				}
			}
			return result;
		}

		template <BMI_VERSION version>
		PATTERN16_FORCE_INLINE auto _pext_u32_BMI(uint32_t a, uint32_t mask) {
			if constexpr (version == BMI2) return _pext_u32(a, mask);
			else if constexpr (version == BMI1) return _pext_u32_BMI1(a, mask);
			else return _pext_u32_BMI_NONE(a, mask);
		}

		PATTERN16_NO_INLINE auto _pdep_u32_BMI_NONE(uint32_t a, uint32_t mask) {
			auto result = 0u;
			auto nmask = ~mask;
			while (mask) {
				auto start = _tzcnt_u32(mask);
				auto bitmask = ~0u << start;
				if (nmask &= bitmask) {
					auto len = _tzcnt_u32(nmask) - start;
					auto bitmask2 = bitmask << len;
					mask &= bitmask2;
					bitmask -= bitmask2;
					result |= (a << start) & bitmask;
					a >>= len;
				}
				else {
					auto bitmask2 = bitmask << -start;
					bitmask -= bitmask2;
					result |= (a << start) & bitmask;
					break;
				}
			}
			return result;
		}

		template <BMI_VERSION version>
		PATTERN16_FORCE_INLINE auto _pdep_u32_BMI(uint32_t a, uint32_t mask) {
			if constexpr (version == BMI2) return _pdep_u32(a, mask);
			else return _pdep_u32_BMI_NONE(a, mask);
		}
	}
}