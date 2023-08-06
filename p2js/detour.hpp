#pragma once

#include <MinHook.h>
#include <vector>
#include "logger.hpp"
#include "patch.hpp"

#define HOOK_THISCALL(type, name, ...) \
	struct name { \
		using Function = type(__fastcall)(__VA_ARGS__); \
		static Function Callback; \
		static inline Function* Original; \
	};

#define HOOK_STDCALL(type, name, ...) \
	struct name { \
		using Function = type(__stdcall)(__VA_ARGS__); \
		static Function Callback; \
		static inline Function* Original; \
	};

#define HOOK_CDECL(type, name, ...) \
	struct name { \
		using Function = type(__cdecl)(__VA_ARGS__); \
		static Function Callback; \
		static inline Function* Original; \
	};

class Detour : public Patch {
public:
	Detour(void* target) : target(target) { }

	~Detour() {
		for(auto address : this->addresses) {
			MH_RemoveHook(address);
		}
	}

	template<typename HookStruct> void Hook() {
		MH_CreateHook(this->target, reinterpret_cast<void*>(&HookStruct::Callback), reinterpret_cast<void**>(&HookStruct::Original));
		MH_QueueEnableHook(this->target);
		this->addresses.push_back(this->target);
	}

	template<typename HookStruct> void Hook(int vmtIndex) {
		auto address = (*reinterpret_cast<void***>(this->target))[vmtIndex];
		MH_CreateHook(address, reinterpret_cast<void*>(&HookStruct::Callback), reinterpret_cast<void**>(&HookStruct::Original));
		MH_QueueEnableHook(address);
		this->addresses.push_back(address);
	}

	void Apply() {
		MH_ApplyQueued();
	}

	void* target;
	std::vector<void*> addresses;
};
