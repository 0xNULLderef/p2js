#include "memory.hpp"

#include <Windows.h>
#include <Psapi.h>
#include <immintrin.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <span>

inline const std::span<uint8_t> GetModuleSpan(std::string moduleName) {
	auto moduleHandle = GetModuleHandleA(moduleName.c_str());
	MODULEINFO moduleInfo = { };
	GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(moduleInfo));
	return { reinterpret_cast<uint8_t*>(moduleInfo.lpBaseOfDll), static_cast<size_t>(moduleInfo.SizeOfImage) };
}

inline const std::pair<const std::vector<uint8_t>, const std::vector<uint8_t>> PrepareSignature(std::string signatureString) {
	std::vector<uint8_t> pattern;
	std::vector<uint8_t> mask;

	std::istringstream signatureStream(signatureString);
	std::string signatureByte;

	while(signatureStream >> signatureByte) {
		if(signatureByte == "?" || signatureByte == "??") {
			pattern.push_back(0x00);
			mask.push_back(0x00);
		} else {
			pattern.push_back(static_cast<uint8_t>(std::stoul(signatureByte, nullptr, 16)));
			mask.push_back(0xFF);
		}
	}

	return { pattern, mask };
}

inline bool MaskedInnerCompare(std::span<uint8_t> block, std::vector<uint8_t> pattern, std::vector<uint8_t> mask) {
	const auto blockSize = block.size();
	if(blockSize != pattern.size() || blockSize != mask.size()) {
		throw std::runtime_error("Block, pattern and mask size do not match");
	}

	// ignore first and last, we don't need them anyway
	for(size_t i = 1; i < blockSize - 1; i++) {
		if((block[i] & mask[i]) != pattern[i]) {
			return false;
		}
	}

	return true;
}

uint8_t* Memory::ScanInternal(std::string moduleName, std::string signatureString) {
	const auto moduleSpan = GetModuleSpan(moduleName);
	const auto [pattern, mask] = PrepareSignature(signatureString);

	if(mask[0] == 0x00 || mask[mask.size() - 1] == 0x00) {
		throw std::runtime_error("A signature cannot begin or and with a wildcard");
	}

	const __m256i firstByteMask = _mm256_set1_epi8(static_cast<char>(pattern.front()));
	const __m256i lastByteMask = _mm256_set1_epi8(static_cast<char>(pattern.back()));

	for(size_t offset = 0; offset < moduleSpan.size() - pattern.size() - 32; offset += 32) {
		const auto scanFirst = &moduleSpan[offset];
		const auto scanLast = &moduleSpan[offset + pattern.size() - 1];

		const auto blockFirst = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(scanFirst));
		const auto blockLast = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(scanLast));

		const auto eqFirst = _mm256_cmpeq_epi8(firstByteMask, blockFirst);
		const auto eqLast = _mm256_cmpeq_epi8(lastByteMask, blockLast);

		uint32_t comparedMask = _mm256_movemask_epi8(_mm256_and_si256(eqFirst, eqLast));

		while(comparedMask != 0) {
			const auto bitPosition = std::countr_zero(comparedMask);
			const auto matchSpan = std::span(&scanFirst[bitPosition], pattern.size());

			if(MaskedInnerCompare(matchSpan, pattern, mask)) {
				return matchSpan.data();
			}

			comparedMask &= comparedMask - 1;
		}
	}

	throw std::runtime_error("Failed to find signature");
}
