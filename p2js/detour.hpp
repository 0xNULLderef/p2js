#pragma once

#include <subhook.h>
#include <vector>
#include "logger.hpp"
#include "patch.hpp"

#ifndef _WIN32
#define __fastcall __attribute__((fastcall))
#define __cdecl __attribute__((cdecl))
#endif

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
		for(auto hook : this->hooks) {
			subhook_remove(hook);
			subhook_free(hook);
		}
	}

	template<typename HookStruct> void Hook() {
		auto hook = subhook_new(this->target, reinterpret_cast<void*>(&HookStruct::Callback), SUBHOOK_TRAMPOLINE);
		subhook_install(hook);
		*reinterpret_cast<void**>(&HookStruct::Original) = subhook_get_trampoline(hook);
		this->hooks.push_back(hook);
	}

	template<typename HookStruct> void Hook(int vmtIndex) {
		auto address = (*reinterpret_cast<void***>(this->target))[vmtIndex];
		auto hook = subhook_new(address, reinterpret_cast<void*>(&HookStruct::Callback), SUBHOOK_TRAMPOLINE);
		subhook_install(hook);
		*reinterpret_cast<void**>(&HookStruct::Original) = subhook_get_trampoline(hook);
		this->hooks.push_back(hook);
	}

	void* target;
	std::vector<subhook_t> hooks;
};
