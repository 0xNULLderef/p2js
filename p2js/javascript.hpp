#pragma once

#include "vscript.hpp"

#include "javascriptconsole.hpp"

class JavaScriptVM : public IScriptVM {
public:
	JavaScriptVM();
	~JavaScriptVM();

	bool Init();
	void Shutdown();
	bool ConnectDebugger();
	void DisconnectDebugger();
	ScriptLanguage_t GetLanguage();
	const char* GetLanguageName();
	void AddSearchPath(const char* pszSearchPath);
	bool Frame(float simTime);
	ScriptStatus_t Run(HSCRIPT hScript, bool bWait);
	ScriptStatus_t Run(HSCRIPT hScript, HSCRIPT hScope = nullptr, bool bWait = true);
	ScriptStatus_t Run(const char* pszScript, bool bWait = true);
	HSCRIPT CompileScript(const char* pszScript, const char* pszId = nullptr);
	void ReleaseScript(HSCRIPT);
	HSCRIPT CreateScope(const char* pszScope, HSCRIPT hParent = nullptr);
	void ReleaseScope(HSCRIPT hScript);
	HSCRIPT LookupFunction(const char* pszFunction, HSCRIPT hScope = nullptr);
	void ReleaseFunction(HSCRIPT hScript);
	ScriptStatus_t ExecuteFunction(HSCRIPT hFunction, ScriptVariant_t* pArgs, int nArgs, ScriptVariant_t* pReturn, HSCRIPT hScope, bool bWait);
	void RegisterFunction(ScriptFunctionBinding_t* pScriptFunction);
	bool RegisterClass(ScriptClassDesc_t* pClassDesc);
	HSCRIPT RegisterInstance(ScriptClassDesc_t* pDesc, void* pInstance);
	void SetInstanceUniqeId(HSCRIPT hInstance, const char* pszId);
	void RemoveInstance(HSCRIPT);
	void* GetInstanceValue(HSCRIPT hInstance, ScriptClassDesc_t* pExpectedType = nullptr);
	bool GenerateUniqueKey(const char* pszRoot, char* pBuf, int nBufSize);
	bool ValueExists(HSCRIPT hScope, const char* pszKey);
	bool SetValue(HSCRIPT hScope, const char* pszKey, const ScriptVariant_t& value);
	bool SetValue(HSCRIPT hScope, const char* pszKey, const char* pszValue);
	void CreateTable(ScriptVariant_t& Table);
	int	GetNumTableEntries(HSCRIPT hScope);
	int GetKeyValue(HSCRIPT hScope, int nIterator, ScriptVariant_t* pKey, ScriptVariant_t* pValue);
	bool GetValue(HSCRIPT hScope, const char* pszKey, ScriptVariant_t* pValue);
	void ReleaseValue(ScriptVariant_t& value);
	bool ClearValue(HSCRIPT hScope, const char* pszKey);
	void WriteState(CUtlBuffer* pBuffer);
	void ReadState(CUtlBuffer* pBuffer);
	void RemoveOrphanInstances();
	void DumpState();
	void SetOutputCallback(ScriptOutputFunc_t pFunc);
	void SetErrorCallback(ScriptErrorFunc_t pFunc);
	bool RaiseException(const char* pszExceptionText);

private:
	// V8
	v8::Isolate::CreateParams createParams;
	v8::Isolate* isolate;
	v8::Global<v8::Context> globalContext;

	// mine
	JavaScriptConsole* console;
	uint64_t serialNumber;
};

IScriptVM* ScriptCreateJavaScriptVM();
void ScriptDestroyJavaScriptVM(IScriptVM* vm);
