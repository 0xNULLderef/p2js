#include "singleton.hpp"
#include "detour.hpp"
#include "patch.hpp"
#include "vscript.hpp"

class Hooks : public Singleton<Hooks> {
public:
	void Initialize();
	void Shutdown();

#ifdef _WIN32
	HOOK_THISCALL(IScriptVM*, CreateVM, void*, void*, ScriptLanguage_t);
	HOOK_THISCALL(void, DestroyVM, void*, void*, IScriptVM*);
#else
	HOOK_CDECL(IScriptVM*, CreateVM, void*, ScriptLanguage_t);
	HOOK_CDECL(void, DestroyVM, void*, IScriptVM*);
#endif
	HOOK_CDECL(HSCRIPT, VScriptCompileScript, const char*, bool);

	std::vector<Patch*> patches;
};
