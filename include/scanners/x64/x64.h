#pragma once

#include <vector>
#include <cstdint>
#include <immintrin.h>

#include "../../util.h"

namespace Pattern16 {
	namespace Impl {
		PATTERN16_NO_INLINE const void* scanRegion(const void* regionStart, const void* regionEnd, size_t sigStart, int, SplitSignature<uint64_t>& signature, size_t length) {
			auto psig = signature.first.data();
			auto pmask = signature.first.data();
			auto psig_bytes = (reinterpret_cast<uint8_t*>(signature.first.data()));
			auto pmask_bytes = (reinterpret_cast<uint8_t*>(signature.second.data()));
			auto sig_bytemask = broadcastMask64(psig_bytes[0 + sigStart], psig_bytes[1 + sigStart]);
			auto sig_offset = -static_cast<intptr_t>(sigStart);
			auto mask_bytemask1 = 0xFFFF'0000'0000ull;
			auto mask_bytemask2 = 0xFFFF'0000'0000'0000ull;
			auto cur = reinterpret_cast<const uint64_t*>(regionStart) - 8;
			auto safety = (signature.first.size() + 5) * sizeof(uint64_t);
			auto span = ((reinterpret_cast<uintptr_t>(regionEnd) - reinterpret_cast<uintptr_t>(regionStart) - safety) >> 6);
			{
			outer_loop_continue1:
			outer_loop_continue2:
				cur += 8;
				if PATTERN16_LIKELY (--span) {
					_mm_prefetch(reinterpret_cast<const char*>(cur + 64), _MM_HINT_T0);
					_mm_prefetch(reinterpret_cast<const char*>(cur + 192), _MM_HINT_T0);
					uint32_t resultl = 0;
					uint64_t resulth = 0;
					{
						auto read_alignedl = sig_bytemask ^ cur[0];
						auto read_alignedh = sig_bytemask ^ cur[4];
						auto read_alignedlh = static_cast<uint32_t>(read_alignedl >> 32);
						auto read_alignedhh = static_cast<uint32_t>(read_alignedh >> 32);
						{
							auto val = 1u << 6;
							resultl |= read_alignedl & mask_bytemask1 ? resultl : val;
							resulth |= read_alignedh & mask_bytemask1 ? resulth : val;
						}
						{
							auto val = 1u << 4;
							resultl |= read_alignedl & mask_bytemask2 ? resultl : val;
							resulth |= read_alignedh & mask_bytemask2 ? resulth : val;
						}
						{
							auto val = 1u << 2;
							resultl |= static_cast<uint32_t>(read_alignedl) & 0xFFFF'0000u ? resultl : val;
							resulth |= static_cast<uint32_t>(read_alignedh) & 0xFFFF'0000u ? resulth : val;
						}
						{
							resultl |= static_cast<uint8_t>(!bool(static_cast<uint16_t>(read_alignedl)));
							resulth |= static_cast<uint8_t>(!bool(static_cast<uint16_t>(read_alignedh)));
						}
					}
					{
						auto read_alignedl = sig_bytemask ^ cur[1];
						auto read_alignedh = sig_bytemask ^ cur[5];
						{
							auto val = 1u << 14;
							resultl |= read_alignedl & mask_bytemask1 ? resultl : val;
							resulth |= read_alignedh & mask_bytemask1 ? resulth : val;
						}
						{
							auto val = 1u << 12;
							resultl |= read_alignedl & mask_bytemask2 ? resultl : val;
							resulth |= read_alignedh & mask_bytemask2 ? resulth : val;
						}
						{
							auto val = 1u << 10;
							resultl |= static_cast<uint32_t>(read_alignedl) & 0xFFFF'0000u ? resultl : val;
							resulth |= static_cast<uint32_t>(read_alignedh) & 0xFFFF'0000u ? resulth : val;
						}
						{
							auto val = 1u << 8;
							resultl |= static_cast<uint16_t>(read_alignedl) ? resultl : val;
							resulth |= static_cast<uint16_t>(read_alignedh) ? resulth : val;
						}
					}
					{
						auto read_alignedl = sig_bytemask ^ cur[2];
						auto read_alignedh = sig_bytemask ^ cur[6];
						{
							auto val = 1u << 22;
							resultl |= read_alignedl & mask_bytemask1 ? resultl : val;
							resulth |= read_alignedh & mask_bytemask1 ? resulth : val;
						}
						{
							auto val = 1u << 20;
							resultl |= read_alignedl & mask_bytemask2 ? resultl : val;
							resulth |= read_alignedh & mask_bytemask2 ? resulth : val;
						}
						{
							auto val = 1u << 18;
							resultl |= static_cast<uint32_t>(read_alignedl) & 0xFFFF'0000u ? resultl : val;
							resulth |= static_cast<uint32_t>(read_alignedh) & 0xFFFF'0000u ? resulth : val;
						}
						{
							auto val = 1u << 16;
							resultl |= static_cast<uint16_t>(read_alignedl) ? resultl : val;
							resulth |= static_cast<uint16_t>(read_alignedh) ? resulth : val;
						}
					}
					{
						auto read_alignedl = sig_bytemask ^ cur[3];
						auto read_alignedh = sig_bytemask ^ cur[7];
						{
							auto val = 1u << 30;
							resultl |= read_alignedl & mask_bytemask1 ? resultl : val;
							resulth |= read_alignedh & mask_bytemask1 ? resulth : val;
						}
						{
							auto val = 1u << 28;
							resultl |= read_alignedl & mask_bytemask2 ? resultl : val;
							resulth |= read_alignedh & mask_bytemask2 ? resulth : val;
						}
						{
							auto val = 1u << 26;
							resultl |= static_cast<uint32_t>(read_alignedl) & 0xFFFF'0000u ? resultl : val;
							resulth |= static_cast<uint32_t>(read_alignedh) & 0xFFFF'0000u ? resulth : val;
						}
						{
							auto val = 1u << 24;
							resultl |= static_cast<uint16_t>(read_alignedl) ? resultl : val;
							resulth |= static_cast<uint16_t>(read_alignedh) ? resulth : val;
						}
					}
					{
						auto read_unalignedl = sig_bytemask ^ reinterpret_cast<const uint64_t*>(reinterpret_cast<const uint8_t*>(cur) + 1)[0];
						auto read_unalignedh = sig_bytemask ^ reinterpret_cast<const uint64_t*>(reinterpret_cast<const uint8_t*>(cur) + 1)[4];
						{
							auto val = 2u << 6;
							resultl = read_unalignedl & mask_bytemask1 ? resultl : val;
							resulth = read_unalignedh & mask_bytemask1 ? resulth : val;
						}
						{
							auto val = 2u << 4;
							resultl |= read_unalignedl & mask_bytemask2 ? resultl : val;
							resulth |= read_unalignedh & mask_bytemask2 ? resulth : val;
						}
						{
							auto val = 2u << 2;
							resultl |= static_cast<uint32_t>(read_unalignedl) & 0xFFFF'0000u ? resultl : val;
							resulth |= static_cast<uint32_t>(read_unalignedh) & 0xFFFF'0000u ? resulth : val;
						}
						{
							auto val = 2u;
							resultl |= static_cast<uint16_t>(read_unalignedl) ? resultl : val;
							resulth |= static_cast<uint16_t>(read_unalignedh) ? resulth : val;
						}
					}
					{
						auto read_unalignedl = sig_bytemask ^ reinterpret_cast<const uint64_t*>(reinterpret_cast<const uint8_t*>(cur) + 1)[1];
						auto read_unalignedh = sig_bytemask ^ reinterpret_cast<const uint64_t*>(reinterpret_cast<const uint8_t*>(cur) + 1)[5];
						{
							auto val = 2u << 14;
							resultl |= read_unalignedl & mask_bytemask1 ? resultl : val;
							resulth |= read_unalignedh & mask_bytemask1 ? resulth : val;
						}
						{
							auto val = 2u << 12;
							resultl |= read_unalignedl & mask_bytemask2 ? resultl : val;
							resulth |= read_unalignedh & mask_bytemask2 ? resulth : val;
						}
						{
							auto val = 2u << 10;
							resultl |= static_cast<uint32_t>(read_unalignedl) & 0xFFFF'0000u ? resultl : val;
							resulth |= static_cast<uint32_t>(read_unalignedh) & 0xFFFF'0000u ? resulth : val;
						}
						{
							auto val = 2u << 8;
							resultl |= static_cast<uint16_t>(read_unalignedl) ? resultl : val;
							resulth |= static_cast<uint16_t>(read_unalignedh) ? resulth : val;
						}
					}
					{
						auto read_unalignedl = sig_bytemask ^ reinterpret_cast<const uint64_t*>(reinterpret_cast<const uint8_t*>(cur) + 1)[2];
						auto read_unalignedh = sig_bytemask ^ reinterpret_cast<const uint64_t*>(reinterpret_cast<const uint8_t*>(cur) + 1)[6];
						{
							auto val = 2u << 22;
							resultl |= read_unalignedl & mask_bytemask1 ? resultl : val;
							resulth |= read_unalignedh & mask_bytemask1 ? resulth : val;
						}
						{
							auto val = 2u << 20;
							resultl |= read_unalignedl & mask_bytemask2 ? resultl : val;
							resulth |= read_unalignedh & mask_bytemask2 ? resulth : val;
						}
						{
							auto val = 2u << 18;
							resultl |= static_cast<uint32_t>(read_unalignedl) & 0xFFFF'0000u ? resultl : val;
							resulth |= static_cast<uint32_t>(read_unalignedh) & 0xFFFF'0000u ? resulth : val;
						}
						{
							auto val = 2u << 16;
							resultl |= static_cast<uint16_t>(read_unalignedl) ? resultl : val;
							resulth |= static_cast<uint16_t>(read_unalignedh) ? resulth : val;
						}
					}
					{
						auto read_unalignedl = sig_bytemask ^ reinterpret_cast<const uint64_t*>(reinterpret_cast<const uint8_t*>(cur) + 1)[3];
						auto read_unalignedh = sig_bytemask ^ reinterpret_cast<const volatile uint64_t*>(reinterpret_cast<const uint8_t*>(cur) + 1)[7];
						{
							auto val = 2u << 30;
							resultl |= read_unalignedl & mask_bytemask1 ? resultl : val;
							resulth |= read_unalignedh & mask_bytemask1 ? resulth : val;
						}
						{
							auto val = 2u << 28;
							resultl |= read_unalignedl & mask_bytemask2 ? resultl : val;
							resulth |= read_unalignedh & mask_bytemask2 ? resulth : val;
						}
						{
							auto val = 2u << 26;
							resultl |= static_cast<uint32_t>(read_unalignedl) & 0xFFFF'0000u ? resultl : val;
							resulth |= static_cast<uint32_t>(read_unalignedh) & 0xFFFF'0000u ? resulth : val;
						}
						{
							auto val = 2u << 24;
							resultl |= static_cast<uint16_t>(read_unalignedl) ? resultl : val;
							resulth |= static_cast<uint16_t>(read_unalignedh) ? resulth : val;
						}
						resulth <<= 32;
						if PATTERN16_LIKELY (!(resulth |= resultl)) goto outer_loop_continue1;
					}
					{
						auto cur_sig = reinterpret_cast<const uint8_t*>(cur) + sig_offset;
						uint64_t result = resulth;
					inner_loop_continue1:
					inner_loop_continue2:
						if PATTERN16_UNLIKELY (!(resulth &= result)) goto outer_loop_continue2;
						auto cur_sig_start = reinterpret_cast<const uint64_t*>(cur_sig + _tzcnt_u64(resulth));
						result = resulth--;
						auto potential_match = *cur_sig_start;
						potential_match ^= psig[0];
						if PATTERN16_LIKELY (potential_match &= signature.second[0]) goto inner_loop_continue1;
						auto length_ = length;
						while (length_--) {
							if (!length_) return (const void*)cur_sig_start;
							auto potential_match = cur_sig_start[length_];
							potential_match ^= psig[length_];
							if (potential_match &= signature.second[length_]) break;
						}
						goto inner_loop_continue2;
					}
				}
			}
			auto cur_byte = reinterpret_cast<const uint8_t*>(cur);
			auto end_byte = reinterpret_cast<const uint8_t*>(cur + 13);
			do {
				auto cur_sig_start = reinterpret_cast<const uint64_t*>(cur_byte);
				auto length_ = length;
				while (length_--) {
					auto potential_match = cur_sig_start[length_];
					potential_match ^= psig[length_];
					if (potential_match & signature.second[length_]) break;
					if (!length_) return (const void*)cur_sig_start;
				}
			} while (++cur_byte < end_byte);
			return nullptr;
		}
	}
}