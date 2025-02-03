#pragma once

#include <stdint.h>
#include <string>

namespace LABN {

constexpr static char kLABNId[4] = { 'L', 'A', 'B', 'N' };

void decompress(const std::string& labPath, const std::string& outFolder);

} // namespace LABN
