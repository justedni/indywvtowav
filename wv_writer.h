#pragma once

#include <stdint.h>
#include <sstream>

namespace WavFormat
{
	struct WavHeader;
}

namespace WvWriter {

	void write(std::string& path, const WavFormat::WavHeader* wavHeader, char* inData, uint32_t compressedSize);

} // namespace WvWriter
