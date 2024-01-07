#pragma once

#include <vector>
#include <cstdint>
#include <immintrin.h>

#include "../../util.h"

namespace Pattern16 {
	namespace Impl {
		template <SSE_VERSION version = SSE4_1>
		PATTERN16_NO_INLINE const void* scanRegion(const void* regionStart, const void* regionEnd, size_t sigStart, int, SplitSignature<__m128i>& signature, size_t length) {
			auto sig_bytes = _mm_load_si128(signature.first.data());
			auto mask_bytes = _mm_load_si128(signature.second.data());
			auto psig_bytes = (reinterpret_cast<uint8_t*>(signature.first.data()));
			auto sig_bytemask = broadcastMask128(psig_bytes[0 + sigStart], psig_bytes[1 + sigStart]);
			auto sig_offset = -(intptr_t)sigStart;
			auto checkmask_unaligned = broadcastMask128(0, 0xFF);
			auto cur = reinterpret_cast<const __m128i*>(reinterpret_cast<const uint8_t*>(regionStart)) - 4;
			auto safety = (signature.first.size() + 3) * sizeof(__m128i);
			auto span = (((uintptr_t)regionEnd - (uintptr_t)regionStart - safety) >> 6);
			{
			outer_loop_continue1:
			outer_loop_continue2:
				cur += 4;
				if PATTERN16_LIKELY (--span) {
					_mm_prefetch(reinterpret_cast<const char*>(cur + 32), _MM_HINT_T0);
					_mm_prefetch(reinterpret_cast<const char*>(cur + 96), _MM_HINT_T0);
					uint64_t result;
					{
						auto read_aligned1 = _mm_load_si128(cur + 2);
						auto read_unaligned1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(reinterpret_cast<const uint8_t*>(cur) + 1) + 2);
						auto read_aligned2 = _mm_load_si128(cur + 3);
						auto read_unaligned2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(reinterpret_cast<const uint8_t*>(cur) + 1) + 3);
						auto check_aligned2 = _mm_cmpeq_epi16(read_aligned2, sig_bytemask);
						auto check_unaligned2 = _mm_cmpeq_epi16(read_unaligned2, sig_bytemask);
						check_unaligned2 = _mm_and_si128(check_unaligned2, checkmask_unaligned);
						result = _mm_movemask_epi8(_mm_or_si128(check_aligned2, check_unaligned2));
						result <<= 16;
						auto check_aligned1 = _mm_cmpeq_epi16(read_aligned1, sig_bytemask);
						auto check_unaligned1 = _mm_cmpeq_epi16(read_unaligned1, sig_bytemask);
						check_unaligned1 = _mm_and_si128(check_unaligned1, checkmask_unaligned);
						result |= _mm_movemask_epi8(_mm_or_si128(check_aligned1, check_unaligned1));
						result <<= 16;
					}
					{
						auto read_aligned1 = _mm_load_si128(cur);
						auto read_unaligned1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(reinterpret_cast<const uint8_t*>(cur) + 1));
						auto read_aligned2 = _mm_load_si128(cur + 1);
						auto read_unaligned2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(reinterpret_cast<const uint8_t*>(cur) + 1) + 1);
						auto check_aligned2 = _mm_cmpeq_epi16(read_aligned2, sig_bytemask);
						auto check_unaligned2 = _mm_cmpeq_epi16(read_unaligned2, sig_bytemask);
						check_unaligned2 = _mm_and_si128(check_unaligned2, checkmask_unaligned);
						result |= _mm_movemask_epi8(_mm_or_si128(check_aligned2, check_unaligned2));
						result <<= 16;
						auto check_aligned1 = _mm_cmpeq_epi16(read_aligned1, sig_bytemask);
						auto check_unaligned1 = _mm_cmpeq_epi16(read_unaligned1, sig_bytemask);
						check_unaligned1 = _mm_and_si128(check_unaligned1, checkmask_unaligned);
						if PATTERN16_LIKELY (!(result |= _mm_movemask_epi8(_mm_or_si128(check_aligned1, check_unaligned1)))) goto outer_loop_continue1;
					}
					{
						auto cur_sig = reinterpret_cast<const uint8_t*>(cur) + sig_offset;
						uint64_t result_ = result;
					inner_loop_continue1:
					inner_loop_continue2:
						if PATTERN16_UNLIKELY (!(result &= result_)) goto outer_loop_continue2;
						auto cur_sig_start = reinterpret_cast<const __m128i*>(cur_sig + _tzcnt_u64(result));
						result_ = result--;
						auto potential_match = _mm_loadu_si128(cur_sig_start);
						potential_match = _mm_xor_si128(potential_match, sig_bytes);
						if PATTERN16_LIKELY (!_mm_testz_SSE<version>(potential_match, mask_bytes)) goto inner_loop_continue1;
						auto length_ = length;
						while (length_--) {
							if (!length_) return (const void*)cur_sig_start;
							auto potential_match = _mm_loadu_si128(cur_sig_start + length_);
							potential_match = _mm_xor_si128(potential_match, signature.first[length_]);
							if (!_mm_testz_SSE<version>(potential_match, signature.second[length_])) break;
						}
						goto inner_loop_continue2;
					}
				}
			}
			auto cur_byte = reinterpret_cast<const uint8_t*>(cur);
			auto end_byte = reinterpret_cast<const uint8_t*>(cur + 7);
			do {
				auto cur_sig_start = reinterpret_cast<const __m128i*>(cur_byte);
				auto length_ = length;
				while (length_--) {
					auto potential_match = _mm_loadu_si128(cur_sig_start + length_);
					potential_match = _mm_xor_si128(potential_match, signature.first[length_]);
					if (!_mm_testz_si128(potential_match, signature.second[length_])) break;
					if (!length_) return (const void*)cur_sig_start;
				}
			} while (++cur_byte < end_byte);
			return nullptr;
		}
	}
}