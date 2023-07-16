#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "../thirdparty/mem/mem.h"
#include "../thirdparty/mem/pattern.h"
#include "../thirdparty/mem/simd_scanner.h"

#include "../../include/Pattern16.h"

const void* memscan(const void* regionStart, size_t regionSize, Pattern16::Impl::SplitSignatureU8& signature, const Pattern16::Impl::Frequencies16& frequencies) {
    auto& [sig, mask] = signature;
    auto pattern = mem::pattern(reinterpret_cast<char*>(sig.data()), reinterpret_cast<char*>(mask.data()), static_cast<mem::pattern::wildcard_t>('?'));
    auto mregion = mem::region(regionStart, regionSize);
    return mem::simd_scanner(pattern).scan(mregion).any();
}