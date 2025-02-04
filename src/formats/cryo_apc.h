#pragma once

#include <string>

namespace CryoAPC {

constexpr static char kAPCTag[8] = { 'C', 'R', 'Y', 'O', '_', 'A', 'P', 'C' };

void apc_to_wav(const std::string& in_WvPath, const std::string& in_outFilePath);

} // namespace CryoAPC
