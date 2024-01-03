#pragma once

#include <map>
#include <array>
#include <memory>
#include <cstdint>
#include <fstream>
#include <immintrin.h>

namespace Pattern16 {
	namespace Impl {
		using Frequencies = std::array<uint32_t, 0x10000>;
		using FrequencyCache = std::unique_ptr<std::array<uint16_t, 0x10000>>;
		using freq12_t = const uint8_t[0x3000];

#include "pfreq.inl"

		inline auto getFrequencies16(const void* regionStart, const void* regionEnd, Frequencies& frequencies) {
			auto cur = reinterpret_cast<const uint64_t*>(regionStart);
			auto end = reinterpret_cast<const uint64_t*>(reinterpret_cast<const uint8_t*>(regionEnd) - 1);
			auto base = frequencies.data();
			while ((uintptr_t)cur < (uintptr_t)end) {
				++base[*reinterpret_cast<const uint16_t*>(cur)];
				++base[*reinterpret_cast<const uint16_t*>(cur + 1)];
				++base[*reinterpret_cast<const uint16_t*>(cur + 2)];
				++base[*reinterpret_cast<const uint16_t*>(cur + 3)];
				++base[*reinterpret_cast<const uint16_t*>(cur + 4)];
				++base[*reinterpret_cast<const uint16_t*>(cur + 5)];
				++base[*reinterpret_cast<const uint16_t*>(cur + 6)];
				++base[*reinterpret_cast<const uint16_t*>(cur + 7)];
				++cur;
			}
		}

		template <size_t count>
		class alignas(64) CacheSerialized {
			static constexpr size_t size_ = count * 3 / 2 + (-static_cast<int>(count * 3 / 2) & 47);
			uint8_t underlying[size_]{};
			template <typename T>
			class Entry {
				using mem_type_u8 = std::conditional_t<std::is_const_v<std::remove_pointer_t<T>>, const uint8_t*, uint8_t*>;
				mem_type_u8 mem;
				uint32_t index;
			public:
				Entry(T mem, uint32_t index) : mem(reinterpret_cast<mem_type_u8>(mem)), index(index) {}

				auto& operator = (uint16_t val) noexcept {
					auto& base = this->getBase();
					if (this->index % 2) {
						base &= 0b0000'0000'0000'1111;
						base |= val << 4;
					}
					else {
						base &= 0b1111'0000'0000'0000;
						base |= val & 0b0000'1111'1111'1111;
					}
					return *this;
				}

				operator int() const noexcept {
					auto& base = this->getBase();
					if (this->index % 2) {
						return base >> 4;
					}
					else {
						return base & 0b0000'1111'1111'1111;
					}
				}

			private:
				auto& getBase() noexcept {
					return *reinterpret_cast<uint16_t*>(this->mem + this->index + this->index / 2);
				}

				auto& getBase() const noexcept {
					return *reinterpret_cast<const uint16_t*>(this->mem + this->index + this->index / 2);
				}
			};
		public:
			constexpr auto size() const noexcept {
				return size_;
			}

			auto data() noexcept {
				return this->underlying;
			}

			const auto data() const noexcept {
				return this->underlying;
			}

			auto operator[] (int index) noexcept {
				return Entry<uint8_t*>(this->underlying, index);
			}

			auto operator[] (int index) const noexcept {
				return Entry<const uint8_t*>(this->underlying, index);
			}
		};
		using Frequencies16 = CacheSerialized<0x2000>;

		template <BMI_VERSION version>
		static auto makeFrequencyCache12_t(const Frequencies& frequencies, Frequencies16& out) {
			std::multimap<uint64_t, uint16_t> frequencyMap;
			for (int i = 0; i < 8192; ++i) {
				auto frequency8 = 0ull;
				auto index = _pdep_u32_BMI<version>(i, ~0b0001'0000'0000'0011);
				frequency8 += static_cast<uint64_t>(frequencies[index | 0]);
				frequency8 += static_cast<uint64_t>(frequencies[index | 1]);
				frequency8 += static_cast<uint64_t>(frequencies[index | 2]);
				frequency8 += static_cast<uint64_t>(frequencies[index | 3]);
				frequency8 += static_cast<uint64_t>(frequencies[index | 0 | 4 << 12]);
				frequency8 += static_cast<uint64_t>(frequencies[index | 1 | 4 << 12]);
				frequency8 += static_cast<uint64_t>(frequencies[index | 2 | 4 << 12]);
				frequency8 += static_cast<uint64_t>(frequencies[index | 3 | 4 << 12]);
				frequencyMap.insert({ frequency8, i });
			}
			auto index = 0u;
			for (auto& frq : frequencyMap) {
				out[frq.second] = index++ >> 1;
			}
			return out;
		}

		inline auto& loadFrequencyCache() {
			return *reinterpret_cast<const Frequencies16*>(frequencies12_t);
		}
	}
}