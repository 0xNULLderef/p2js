#pragma once

#include "singleton.hpp"
#include "logger.hpp"
#include <string>
#include <functional>

class Memory : public Singleton<Memory> {
public:
	// REQUIRES the signature to NOT start and end with a wildcard character
	template<typename T> void Scan(std::string moduleName, std::string signatureString, std::function<void(T)> callback) {
		try {
			auto matchAddress = this->ScanInternal(moduleName, signatureString);
			if(matchAddress != nullptr) {
				callback(reinterpret_cast<T>(matchAddress));
			}
		} catch(std::exception& ex) {
			WARNING("Failed to match signature %s in %s (%s)\n", signatureString.c_str(), moduleName.c_str(), ex.what());
		}
	}

	template<typename T> void Scan(std::string moduleName, std::string signatureString, int offset, std::function<void(T)> callback) {
		try {
			const auto matchAddress = this->ScanInternal(moduleName, signatureString);
			if(matchAddress != nullptr) {
				callback(reinterpret_cast<T>(reinterpret_cast<uintptr_t>(matchAddress) + offset));
			}
		} catch(std::exception& ex) {
			WARNING("Failed to match signature %s in %s (%s)\n", signatureString.c_str(), moduleName.c_str(), ex.what());
		}
	}

	template<typename T> inline void GetInterface(std::string moduleName, std::string interfaceName, std::function<void(T)> callback) {
		auto moduleHandle = GetModuleHandleA(moduleName.c_str());
		auto CreateInterface = reinterpret_cast<void*(*)(const char*, int*)>(GetProcAddress(moduleHandle, "CreateInterface"));
		if(CreateInterface) {
			auto queriedInterface = CreateInterface(interfaceName.c_str(), nullptr);
			if(queriedInterface) { 
				callback(reinterpret_cast<T>(queriedInterface));
			}
		}
	}

private:
	uint8_t* ScanInternal(std::string moduleName, std::string signatureString);
};
