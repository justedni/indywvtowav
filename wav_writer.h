#pragma once

#include <stdint.h>
#include <sstream>

struct IndyWVHeader;

namespace WavWriter {

void write(std::string& path, const IndyWVHeader* wvHeader, char* inData);

} // namespace WavWriter
