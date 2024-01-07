#pragma once

#include <vector>
#include <cstdint>
#include <immintrin.h>

#include "../../util.h"

namespace Pattern16 {
	namespace Impl {
		PATTERN16_NO_INLINE const void* scanRegion(const void* regionStart, const void* regionEnd, size_t sigStart, int, SplitSignature<__m256i>& signature, size_t length) {
			auto sig_bytes = _mm256_load_si256(signature.first.data());
			auto mask_bytes = _mm256_load_si256(signature.second.data());
			auto psig_bytes = (reinterpret_cast<uint8_t*>(signature.first.data()));
			auto sig_bytemask = broadcastMask256(psig_bytes[0 + sigStart], psig_bytes[1 + sigStart]);
			auto sig_offset = -(intptr_t)sigStart;
			auto blendmask = _mm256_set1_epi16(0x8000u);
			auto cur = reinterpret_cast<const __m256i*>(regionStart) - 2;
			auto safety = (signature.first.size() + 1) * sizeof(__m256i);
			auto span = (((uintptr_t)regionEnd - (uintptr_t)regionStart - safety) >> 6);
			{
			outer_loop_continue1:
			outer_loop_continue2:
				cur += 2;
				if PATTERN16_LIKELY (--span) {
					_mm_prefetch(reinterpret_cast<const char*>(cur + 16), _MM_HINT_T0);
					_mm_prefetch(reinterpret_cast<const char*>(cur + 48), _MM_HINT_T0);
					uint64_t result;
					uint32_t resultl;
					uint32_t resulth;
					{
						auto read_aligned1 = _mm256_stream_load_si256(cur);
						auto read_unaligned1 = _mm256_lddqu_si256(reinterpret_cast<const __m256i*>(reinterpret_cast<const uint8_t*>(cur) + 1));
						auto check_aligned1 = _mm256_cmpeq_epi16(sig_bytemask, read_aligned1);
						auto check_unaligned1 = _mm256_cmpeq_epi16(sig_bytemask, read_unaligned1);
						resultl = _mm256_movemask_epi8(_mm256_blendv_epi8(check_aligned1, check_unaligned1, blendmask));
						auto read_aligned2 = _mm256_stream_load_si256(cur + 1);
						auto read_unaligned2 = _mm256_lddqu_si256(reinterpret_cast<const __m256i*>(reinterpret_cast<const uint8_t*>(cur) + 1) + 1);
						auto check_aligned2 = _mm256_cmpeq_epi16(sig_bytemask, read_aligned2);
						auto check_unaligned2 = _mm256_cmpeq_epi16(sig_bytemask, read_unaligned2);
						resulth = _mm256_movemask_epi8(_mm256_blendv_epi8(check_aligned2, check_unaligned2, blendmask));
						if PATTERN16_LIKELY (!(resulth | resultl)) goto outer_loop_continue1;
					}
					{
						result = resultl;
						result += static_cast<uint64_t>(resulth) << 32;
						auto cur_sig = reinterpret_cast<const uint8_t*>(cur) + sig_offset;
						uint64_t result_ = result;
					inner_loop_continue1:
					inner_loop_continue2:
						if PATTERN16_UNLIKELY (!(result &= result_)) goto outer_loop_continue2;
						auto cur_sig_start = reinterpret_cast<const __m256i*>(cur_sig + _tzcnt_u64(result));
						result_ = result--;
						auto potential_match = _mm256_lddqu_si256(cur_sig_start);
						potential_match = _mm256_xor_si256(potential_match, sig_bytes);
						if PATTERN16_LIKELY (!_mm256_testz_si256(potential_match, mask_bytes)) goto inner_loop_continue1;
						auto length_ = length;
						while (length_--) {
							if (!length_) return (const void*)cur_sig_start;
							auto potential_match = _mm256_lddqu_si256(cur_sig_start + length_);
							potential_match = _mm256_xor_si256(potential_match, signature.first[length_]);
							if (!_mm256_testz_si256(potential_match, signature.second[length_])) break;
						}
						goto inner_loop_continue2;
					}
				}
			}
			auto cur_byte = reinterpret_cast<const uint8_t*>(cur);
			auto end_byte = reinterpret_cast<const uint8_t*>(cur + 3);
			do {
				auto cur_sig_start = reinterpret_cast<const __m256i*>(cur_byte);
				auto length_ = length;
				while (length_--) {
					auto potential_match = _mm256_lddqu_si256(cur_sig_start + length_);
					potential_match = _mm256_xor_si256(potential_match, signature.first[length_]);
					if (!_mm256_testz_si256(potential_match, signature.second[length_])) break;
					if (!length_) return (const void*)cur_sig_start;
				}
			} while (++cur_byte < end_byte);
			return nullptr;
		}
	}
}