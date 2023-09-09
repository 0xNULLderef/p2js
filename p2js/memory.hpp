#pragma once

#include "singleton.hpp"
#include "logger.hpp"
#include <string>
#include <functional>
#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#else
#include <dlfcn.h>
#include <unordered_map>
#endif
#include <span>
#include <sstream>

class Scanner {
public:
	virtual uint8_t* Scan(std::span<uint8_t> region, std::pair<const std::vector<uint8_t>, const std::vector<uint8_t>> signature) = 0;
};

class Memory : public Singleton<Memory> {
public:
	Memory();
	~Memory();

	// REQUIRES the signature to NOT start and end with a wildcard character
	template<typename T> void Scan(std::string moduleName, std::string signatureString, int offset, std::function<void(T)> callback) {
		try {
			const auto matchAddress = this->scanner->Scan(
				this->GetModuleSpan(moduleName),
				this->PrepareSignature(signatureString)
			);
			if(matchAddress != nullptr) {
				callback(reinterpret_cast<T>(reinterpret_cast<uintptr_t>(matchAddress) + offset));
			}
		} catch(std::exception& ex) {
			WARNING("Failed to match signature %s in %s (%s)\n", signatureString.c_str(), moduleName.c_str(), ex.what());
		}
	}

	template<typename T> inline void GetInterface(std::string moduleName, std::string interfaceName, std::function<void(T)> callback) {
#ifdef _WIN32
		auto moduleHandle = GetModuleHandleA(moduleName.c_str());
		auto CreateInterface = reinterpret_cast<void*(*)(const char*, int*)>(GetProcAddress(moduleHandle, "CreateInterface"));
#else
		auto moduleHandle = dlopen(moduleName.c_str(), RTLD_NOLOAD | RTLD_NOW);
		auto CreateInterface = reinterpret_cast<void*(*)(const char*, int*)>(dlsym(moduleHandle, "CreateInterface"));
#endif
		if(CreateInterface) {
			auto queriedInterface = CreateInterface(interfaceName.c_str(), nullptr);
			if(queriedInterface) { 
				callback(reinterpret_cast<T>(queriedInterface));
			}
		}

	}

	const std::span<uint8_t> GetModuleSpan(std::string moduleName);

	const std::pair<const std::vector<uint8_t>, const std::vector<uint8_t>> PrepareSignature(std::string signatureString);

#ifndef _WIN32
	static std::unordered_map<std::string, std::span<uint8_t>> moduleSpans;
#endif

private:
	Scanner* scanner;
};
