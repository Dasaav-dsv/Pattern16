# Pattern16
The fastest x86-64 signature matching library.
### Features:
 - Faster than the fastest existing scanners by ~50%, with reliable singlethreaded speeds up to 25 GB/s for consecutive scans
 - Aids reverse engineering by targeting assembly bytecode
 - Designed and optimized for x86-64, with 32-bit support planned
 - Support for all new (and old) CPU features
 - Uses AVX1, SSE4.1, SSE2, CMOVE in order of availability, BMI2 and BMI1 (with fill-in functions)
 - Header only, written in modern C++
### Usage:
Include `Pattern16.h` and provide the address of a memory region's start, its length and the signature to search for formatted as a string:
```cpp
#include "Pattern16.h"

void* regionStart = (void*)0x140000000;
size_t regionSize = 0x4000000;
std::string signature = "00 11 ?? ?? ?? ?? 66 77 [?1?0??01] 99 AA BB C? ?D EE FF";
void* address = Pattern16::find(regionStart, regionSize, signature);
```
Pattern16 signature rules:
 - All byte values are represented in base16/hexadecimal notation
 - Space characters ` ` are ignored completely even inside bit masks, so use them for formatting
 - Symbols other than `0123456789ABCDEFabcdef[]` are wildcards and can stand in for any byte or bit
 - A sequence of symbols within sqare brackets `[]` represents a bit field. Don't forget there are 8 bits in a byte!
 - Bits inside a bitfield can be masked with wildcard symbols
 - A bitfield does not have to be limited to a single byte, `[01xx1100 xxx11xx0]` is a legal 2-byte masked bitfield
