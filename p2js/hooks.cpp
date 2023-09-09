#include "hooks.hpp"

#include "memory.hpp"
#include "bytepatch.hpp"
#include "logger.hpp"
#include "main.hpp"
#include "javascript.hpp"

static const char* extensionArray[] = {
	"", // SL_NONE
	".gm", // SL_GAMEMONKEY
	".nut", // SL_SQUIRREL
	".lua", // SL_LUA
	".py", // SL_PYTHON
	".js", // SL_JAVASCRIPT
};

void Hooks::Initialize() {
	Memory::Instance()->GetInterface<void*>("vscript", "VScriptManager009", [&](void* scriptmanager) {
		auto detour = new Detour(scriptmanager);
		detour->Hook<CreateVM>(8);
		detour->Hook<DestroyVM>(9);
		this->patches.push_back(detour);
	});

	Memory::Instance()->Scan<void*>("server", "BF 02 00 00 00 89 7D FC", 1, [&](void* scriptlang) {
		auto bytepatch = new BytePatch(scriptlang, { SL_JAVASCRIPT });
		this->patches.push_back(bytepatch);
	});

	Memory::Instance()->Scan<void*>("server", "55 8B EC 8B 0D ?? ?? ?? ?? 81 EC", 0, [&](void* compileScript) {
		auto detour = new Detour(compileScript);
		detour->Hook<VScriptCompileScript>();
		this->patches.push_back(detour);
	});

	Memory::Instance()->Scan<void*>("server", "8B 3C 85 ?? ?? ?? ?? 6A 2E", 3, [&](void* extensions) {
		auto bytepatch = new BytePatch(extensions, extensionArray);
		this->patches.push_back(bytepatch);
	});
}

void Hooks::Shutdown() {
	for(auto patch: this->patches) {
		delete patch;
	}
}

IScriptVM* Hooks::CreateVM::Callback(void* thisptr, void*, ScriptLanguage_t language) {
	if(language == SL_JAVASCRIPT) {
		return ScriptCreateJavaScriptVM();
	} else {
		return Original(thisptr, nullptr, language);
	}
}

void Hooks::DestroyVM::Callback(void* thisptr, void*, IScriptVM* vm) {
	if(vm->GetLanguage() == SL_JAVASCRIPT) {
		ScriptDestroyJavaScriptVM(vm);
	} else {
		Original(thisptr, nullptr, vm);
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
