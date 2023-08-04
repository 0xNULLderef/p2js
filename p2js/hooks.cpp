#include "hooks.hpp"

#include "memory.hpp"
#include "bytepatch.hpp"
#include "logger.hpp"
#include "main.hpp"
#include "javascript.hpp"

void Hooks::Initialize() {
	MH_Initialize();

	Memory::Instance()->GetInterface<void*>("vscript", "VScriptManager009", [&](void* scriptmanager) {
		auto detour = new Detour(scriptmanager);
		detour->Hook<CreateVM>(8);
		detour->Hook<DestroyVM>(9);
		detour->Apply();
		this->patches.push_back(detour);
	});

	Memory::Instance()->Scan<void*>("server", "BF 02 00 00 00 89 7D FC", 1, [&](void* scriptlang) {
		auto bytepatch = new BytePatch(scriptlang, { SL_JAVASCRIPT });
		this->patches.push_back(bytepatch);
	});
}

void Hooks::Shutdown() {
	for(auto patch: this->patches) {
		delete patch;
	}

	MH_Uninitialize();
}

IScriptVM* Hooks::CreateVM::Callback(void* thisptr, void*, ScriptLanguage_t language) {
	DEV("CreateVM : %d\n", language);
	if(language == SL_JAVASCRIPT) {
		return ScriptCreateJavaScriptVM();
	} else {
		Original(thisptr, nullptr, language);
	}
}

void Hooks::DestroyVM::Callback(void* thisptr, void*, IScriptVM* vm) {
	DEV("DestroyVM : %d\n", vm->GetLanguage());
	if(vm->GetLanguage() == SL_JAVASCRIPT) {
		ScriptDestroyJavaScriptVM(vm);
	} else {
		Original(thisptr, nullptr, vm);
	}
}
