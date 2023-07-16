#include <iostream>

#include <memory>

#include "include/clfile.h"
#include "include/scan.h"
#include "include/PE.h"

#include "include/memscan.h"
#include "../include/Pattern16.h"

int main(int argc, char* argv[]) {
    auto files = processCL(argc, argv);
    for (auto& file : files) {
        const auto& [name, bytes] = file;
        std::cout << "Processing " << name << "\n";
        auto result = locateTextSections(bytes);
        auto frequencies = std::make_unique<Pattern16::Impl::Frequencies>();
        for (auto& region : result) {
            getRegionFrequencies(region, *frequencies);
        }
        auto serialized = serializeFrequencies(*frequencies);
        cacheFrequencies(name, *serialized);
        timedScan(result, 100, *serialized, Pattern16::Impl::scan);
    }
    system("PAUSE");
    return 0;
}