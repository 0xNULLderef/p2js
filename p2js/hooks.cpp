#include "hooks.hpp"

#include "memory.hpp"
#include "bytepatch.hpp"
#include "logger.hpp"
#include "main.hpp"
#include "javascript.hpp"

#ifdef _WIN32
#define PLATFORM(windows, linux) windows
#else
#define PLATFORM(window, linux) linux
#endif

static const char* extensionArray[] = {
	"", // SL_NONE
	".gm", // SL_GAMEMONKEY
	".nut", // SL_SQUIRREL
	".lua", // SL_LUA
	".py", // SL_PYTHON
	".js", // SL_JAVASCRIPT
};

void Hooks::Initialize() {
	Memory::Instance()->GetInterface<void*>("vscript" PLATFORM(".dll", ".so"), "VScriptManager009", [&](void* scriptmanager) {
		printf("scriptmanager: %p\n", scriptmanager);
		auto detour = new Detour(scriptmanager);
		detour->Hook<CreateVM>(8);
		detour->Hook<DestroyVM>(9);
		this->patches.push_back(detour);
	});

	Memory::Instance()->Scan<void*>("server", PLATFORM("BF 02 00 00 00 89 7D FC", "BD ?? ?? ?? ?? 85 F6 0F 85 E8 EB FF FF"), 1, [&](void* scriptlang) {
		printf("scriptlang: %p\n", scriptlang);
		auto bytepatch = new BytePatch(scriptlang, { SL_JAVASCRIPT });
		this->patches.push_back(bytepatch);
	});

#ifdef _WIN32
	Memory::Instance()->Scan<void*>("server", "55 8B EC 8B 0D ?? ?? ?? ?? 81 EC", 0, [&](void* compileScript) {
		auto detour = new Detour(compileScript);
		detour->Hook<VScriptCompileScript>();
		this->patches.push_back(detour);
	});
#else
	Memory::Instance()->Scan<uintptr_t>("server", "E8 ?? ?? ?? ?? 83 C4 ?? 89 C6 A1 ?? ?? ?? ?? 85 F6", 1, [&](uintptr_t compileScript) {
		printf("0x%x\n", compileScript);
		auto detour = new Detour(reinterpret_cast<void*>(compileScript + *reinterpret_cast<uintptr_t*>(compileScript) + sizeof(compileScript)));
		detour->Hook<VScriptCompileScript>();
		this->patches.push_back(detour);
	});
#endif

	Memory::Instance()->Scan<void*>("server", PLATFORM("8B 3C 85 ?? ?? ?? ?? 6A 2E", "8B 34 85 ?? ?? ?? ?? 58"), 3, [&](void* extensions) {
		printf("extensions: %p\n", extensions);
		auto bytepatch = new BytePatch(extensions, extensionArray);
		this->patches.push_back(bytepatch);
	});
}

void Hooks::Shutdown() {
	for(auto patch: this->patches) {
		delete patch;
	}
}

#ifdef _WIN32
IScriptVM* Hooks::CreateVM::Callback(void* thisptr, void*, ScriptLanguage_t language) {
#else
IScriptVM* Hooks::CreateVM::Callback(void* thisptr, ScriptLanguage_t language) {
#endif
	if(language == SL_JAVASCRIPT) {
		return ScriptCreateJavaScriptVM();
	} else {
#ifdef _WIN32
		return Original(thisptr, nullptr, language);
#else
		return Original(thisptr, language);
#endif
	}
}

#ifdef _WIN32
void Hooks::DestroyVM::Callback(void* thisptr, void*, IScriptVM* vm) {
#else
void Hooks::DestroyVM::Callback(void* thisptr, IScriptVM* vm) {
#endif
	if(vm->GetLanguage() == SL_JAVASCRIPT) {
		ScriptDestroyJavaScriptVM(vm);
	} else {
#ifdef _WIN32
		Original(thisptr, nullptr, vm);
#else
		Original(thisptr, vm);
#endif
	}
}

HSCRIPT Hooks::VScriptCompileScript::Callback(const char* scriptName, bool warnMissing) {
	auto firstDotCharacter = strchr(scriptName, '.');
	if(firstDotCharacter) {
		auto firstDotCharacterIndex = firstDotCharacter - scriptName;
#ifdef _WIN32
		char buffer[MAX_PATH];
		strncpy_s(buffer, scriptName, firstDotCharacterIndex);
#else
		char buffer[PATH_MAX];
		strncpy(buffer, scriptName, firstDotCharacterIndex);
#endif
		buffer[firstDotCharacterIndex] = 0;
		return Original(buffer, warnMissing);
	} else {
		return Original(scriptName, warnMissing);
	}
}
