#include "memory.hpp"

#include <immintrin.h>
#include <vector>
#include <algorithm>
#include <bit>

template<size_t StartOffset, size_t EndOffset> inline bool MaskedCompare(std::span<uint8_t> block, std::vector<uint8_t> pattern, std::vector<uint8_t> mask) {
	const auto blockSize = block.size();
	if(blockSize != pattern.size() || blockSize != mask.size()) {
		throw std::runtime_error("Block, pattern and mask size do not match");
	}

	// ignore first and last, we don't need them anyway
	for(size_t i = StartOffset; i < blockSize - EndOffset; i++) {
		if((block[i] & mask[i]) != pattern[i]) {
			return false;
		}
	}

	return true;
}

class AVXScanner : public Scanner {
public:
	uint8_t* Scan(std::span<uint8_t> region, std::pair<const std::vector<uint8_t>, const std::vector<uint8_t>> signature) {
		const auto [pattern, mask] = signature;

		if(mask[0] == 0x00 || mask[mask.size() - 1] == 0x00) {
			throw std::runtime_error("A signature cannot begin or and with a wildcard");
		}

		const __m256i firstByteMask = _mm256_set1_epi8(static_cast<char>(pattern.front()));
		const __m256i lastByteMask = _mm256_set1_epi8(static_cast<char>(pattern.back()));

		for(size_t offset = 0; offset < region.size() - pattern.size() - 32; offset += 32) {
			const auto scanFirst = &region[offset];
			const auto scanLast = &region[offset + pattern.size() - 1];

			const auto blockFirst = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(scanFirst));
			const auto blockLast = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(scanLast));

			const auto eqFirst = _mm256_cmpeq_epi8(firstByteMask, blockFirst);
			const auto eqLast = _mm256_cmpeq_epi8(lastByteMask, blockLast);

			uint32_t comparedMask = _mm256_movemask_epi8(_mm256_and_si256(eqFirst, eqLast));

			while(comparedMask != 0) {
				const auto bitPosition = std::countr_zero(comparedMask);
				const auto matchSpan = std::span(&scanFirst[bitPosition], pattern.size());

				if(MaskedCompare<1, 1>(matchSpan, pattern, mask)) {
					return matchSpan.data();
				}

				comparedMask &= comparedMask - 1;
			}
		}

		throw std::runtime_error("Failed to find signature");
	}
};

class SSEScanner : public Scanner {
public:
	uint8_t* Scan(std::span<uint8_t> region, std::pair<const std::vector<uint8_t>, const std::vector<uint8_t>> signature) {
		const auto [pattern, mask] = signature;

		if(mask[0] == 0x00 || mask[mask.size() - 1] == 0x00) {
			throw std::runtime_error("A signature cannot begin or and with a wildcard");
		}

		const __m128i firstByteMask = _mm_set1_epi8(static_cast<char>(pattern.front()));
		const __m128i lastByteMask = _mm_set1_epi8(static_cast<char>(pattern.back()));

		for(size_t offset = 0; offset < region.size() - pattern.size() - 16; offset += 16) {
			const auto scanFirst = &region[offset];
			const auto scanLast = &region[offset + pattern.size() - 1];

			const auto blockFirst = _mm_loadu_si128(reinterpret_cast<const __m128i*>(scanFirst));
			const auto blockLast = _mm_loadu_si128(reinterpret_cast<const __m128i*>(scanLast));

			const auto eqFirst = _mm_cmpeq_epi8(firstByteMask, blockFirst);
			const auto eqLast = _mm_cmpeq_epi8(lastByteMask, blockLast);

			uint32_t comparedMask = _mm_movemask_epi8(_mm_and_si128(eqFirst, eqLast));

			while(comparedMask != 0) {
				const auto bitPosition = std::countr_zero(comparedMask);
				const auto matchSpan = std::span(&scanFirst[bitPosition], pattern.size());

				if(MaskedCompare<1, 1>(matchSpan, pattern, mask)) {
					return matchSpan.data();
				}

				comparedMask &= comparedMask - 1;
			}
		}

		throw std::runtime_error("Failed to find signature");
	}
};

class GenericScanner : public Scanner {
public:
	uint8_t* Scan(std::span<uint8_t> region, std::pair<const std::vector<uint8_t>, const std::vector<uint8_t>> signature) {
		const auto [pattern, mask] = signature;

		for(size_t offset = 0; offset < region.size() - pattern.size() - 1; offset += 1) {
			const auto matchSpan = std::span(&region[offset], pattern.size());
			if(MaskedCompare<0, 0>(matchSpan, pattern, mask)) {
				return matchSpan.data();
			}
		}

		throw std::runtime_error("Failed to find signature");
	}
};

template<unsigned int Function, unsigned int Register, unsigned int Bit> inline bool HasCpuId() {
	static_assert(Register >= 0 && Register <= 4, "Invalid register");
	static_assert(Bit >= 0 && Bit <= 32, "Invalid register");
	int cpuInfo[4];
#ifdef _WIN32
	__cpuidex(cpuInfo, Function, 0);
#else
	asm volatile("cpuid"
		: "=a" (cpuInfo[0]),
		  "=b" (cpuInfo[1]),
		  "=c" (cpuInfo[2]),
		  "=d" (cpuInfo[3])
		: "0" (Function), "2" (0)
	);
#endif
	return cpuInfo[Register] & (1 << Bit);
}

const std::span<uint8_t> Memory::GetModuleSpan(std::string moduleName) {
	auto moduleHandle = GetModuleHandleA(moduleName.c_str());
	if(!moduleHandle) { 
		throw std::runtime_error("Failed to get module handle");
	}
	MODULEINFO moduleInfo = { };
	if(!GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(moduleInfo))) {
		throw std::runtime_error("Failed to get module span");
	}
	return { reinterpret_cast<uint8_t*>(moduleInfo.lpBaseOfDll), static_cast<size_t>(moduleInfo.SizeOfImage) };
}

const std::pair<const std::vector<uint8_t>, const std::vector<uint8_t>> Memory::PrepareSignature(std::string signatureString) {
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

Memory::Memory() {
	// AVX is 1, ECX(2) bit 28
	// SSE2 is 1, EDX(3) bit 26
	if(HasCpuId<1, 2, 28>()) {
		INFO("Using AVX Scanner\n");
		this->scanner = new AVXScanner;
	} else if(HasCpuId<1, 3, 26>()) {
		INFO("Using SSE Scanner\n");
		this->scanner = new SSEScanner;
	} else { 
		WARNING("Using SSE Scanner, may be slow!\n");
		this->scanner = new GenericScanner;
	}
}

Memory::~Memory() {
	delete this->scanner;
}
