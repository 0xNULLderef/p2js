#include "singleton.hpp"
#include "detour.hpp"
#include "patch.hpp"
#include "vscript.hpp"

class Hooks : public Singleton<Hooks> {
public:
	void Initialize();
	void Shutdown();

	HOOK_THISCALL(IScriptVM*, CreateVM, void*, void*, ScriptLanguage_t);
	HOOK_THISCALL(void, DestroyVM, void*, void*, IScriptVM*);

	std::vector<Patch*> patches;
};