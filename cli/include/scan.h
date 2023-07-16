#pragma once

#include <cstdint>
#include <vector>

#include <chrono>
#include <random>
#include <immintrin.h>

#include <iostream>

#include "../../include/Pattern16.h"

#define RAND_SIG_SIZE 37
#define ATTEMPT_COUNT 16

using ScannerFn = decltype(Pattern16::Impl::scan);

template <typename result_t = std::chrono::microseconds, typename duration_t = std::chrono::microseconds>
auto since(const std::chrono::time_point<std::chrono::steady_clock, duration_t>& start) {
    return std::chrono::duration_cast<result_t>(std::chrono::steady_clock::now() - start);
}

auto getRandOffset(size_t regionSize) {
    static std::mt19937 mt(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, regionSize - RAND_SIG_SIZE);
    return dist(mt);
}

auto getUniqueSignatures(const std::pair<const void*, size_t>& region, size_t count, const Pattern16::Impl::Frequencies16& frequencies, ScannerFn scanner) {
    const auto& [regionStart, regionSize] = region;
    std::vector<Pattern16::Impl::SplitSignatureU8> signatures{};
    while (count--) {
        signatures.push_back({ {},{} });
        auto& [sig, mask] = signatures.back();
        sig.resize(RAND_SIG_SIZE);
        mask.resize(RAND_SIG_SIZE, 0xFF);
        for (int i = 0; i < ATTEMPT_COUNT; ++i) {
            auto offset = getRandOffset(regionSize);
            std::memcpy((void*)sig.data(), (void*)((uintptr_t)regionStart + offset), RAND_SIG_SIZE);
            auto found = scanner(regionStart, regionSize, signatures.back(), frequencies);
            if ((uintptr_t)found - (uintptr_t)regionStart == offset) break;
            else if (i == ATTEMPT_COUNT - 1) signatures.pop_back();
        }
    }
    return signatures;
}

void timedScan(const std::vector<std::pair<const void*, size_t>>& regions, size_t count, const Pattern16::Impl::Frequencies16& frequencies, ScannerFn scanner) {
    auto t_total = 1ull;
    auto b_total = 0ull;
    for (const auto& region : regions) {
        const auto& [regionStart, regionSize] = region;
        auto signatures = getUniqueSignatures(region, count, frequencies, scanner);
        for (auto& signature : signatures) {
            auto start = std::chrono::steady_clock::now();
            auto result = scanner(regionStart, regionSize, signature, frequencies);
            if (result) {
                t_total += since(start).count();
                b_total += (uintptr_t)result - (uintptr_t)regionStart;
            }
        }
    }
    std::cout << "Did " << count << " iterations in ";
    std::cout << static_cast<double>(t_total) / 1'000'000.0 << " seconds at ";
    std::cout << static_cast<double>(b_total / t_total) / 1'000.0 << "GB/s \n";
}