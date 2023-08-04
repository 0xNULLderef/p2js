#include "javascript.hpp"

#include "logger.hpp"
#include <v8-template.h>
#include <v8-external.h>
#include <v8-function.h>

JavaScriptVM::JavaScriptVM() {
	// V8
	this->createParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	this->isolate = v8::Isolate::New(this->createParams);
	v8::Isolate::Scope isolateScope(this->isolate);
	v8::HandleScope handleScope(this->isolate);
	this->globalContext.Reset(this->isolate, v8::Context::New(this->isolate));

	// mine
	this->console = new JavaScriptConsole(this->isolate);
	v8::debug::SetConsoleDelegate(this->isolate, this->console);
	serialNumber = 0;
}

JavaScriptVM::~JavaScriptVM() {
	// mine
	delete this->console;

	// V8
	this->globalContext.Reset();
	this->isolate->Dispose();
	delete this->createParams.array_buffer_allocator;
}

bool JavaScriptVM::Init() {
	DEV("STUB : %d\n", __LINE__);
	return true;
}

void JavaScriptVM::Shutdown() {
	DEV("STUB : %d\n", __LINE__);
}

bool JavaScriptVM::ConnectDebugger() {
	DEV("STUB : %d\n", __LINE__);
	return false;
}

void JavaScriptVM::DisconnectDebugger() {
	DEV("STUB : %d\n", __LINE__);
}

ScriptLanguage_t JavaScriptVM::GetLanguage() {
	return SL_JAVASCRIPT;
}

const char* JavaScriptVM::GetLanguageName() {
	return "JavaScript";
}

void JavaScriptVM::AddSearchPath(const char* pszSearchPath) {
	DEV("STUB : %d\n", __LINE__);
}

bool JavaScriptVM::Frame(float simTime) {
	return false;
}

ScriptStatus_t JavaScriptVM::Run(const char* pszScript, bool bWait) {
	v8::HandleScope handleScope(this->isolate);
	v8::Local<v8::Context> context = v8::Local<v8::Context>::New(this->isolate, this->globalContext);
	v8::Context::Scope contextScope(context);

	auto source = v8::String::NewFromUtf8(this->isolate, pszScript).ToLocalChecked();
	v8::Local<v8::Script> script;
	if(!v8::Script::Compile(context, source).ToLocal(&script)) return SCRIPT_ERROR;
	script->Run(context).ToLocalChecked();

	return SCRIPT_DONE;
}

ScriptStatus_t JavaScriptVM::Run(HSCRIPT hScript, HSCRIPT hScope, bool bWai) {
	DEV("STUB : %d\n", __LINE__);
	return SCRIPT_ERROR;
}

ScriptStatus_t JavaScriptVM::Run(HSCRIPT hScript, bool bWait) {
	DEV("STUB : %d\n", __LINE__);
	return SCRIPT_ERROR;
}

HSCRIPT JavaScriptVM::CompileScript(const char* pszScript, const char* pszId) {
	DEV("STUB : %d\n", __LINE__);
	return nullptr;
}

void JavaScriptVM::ReleaseScript(HSCRIPT) {
	DEV("STUB : %d\n", __LINE__);
}

HSCRIPT JavaScriptVM::CreateScope(const char* pszScope, HSCRIPT hParent) {
	DEV("STUB : %d\n", __LINE__);
	return nullptr;
}

void JavaScriptVM::ReleaseScope(HSCRIPT hScript) {
	DEV("STUB : %d\n", __LINE__);
}

HSCRIPT JavaScriptVM::LookupFunction(const char* pszFunction, HSCRIPT hScope) {
	DEV("STUB : %d\n", __LINE__);
	return nullptr;
}

void JavaScriptVM::ReleaseFunction(HSCRIPT hScript) {
	DEV("STUB : %d\n", __LINE__);
}

ScriptStatus_t JavaScriptVM::ExecuteFunction(HSCRIPT hFunction, ScriptVariant_t* pArgs, int nArgs, ScriptVariant_t* pReturn, HSCRIPT hScope, bool bWait) {
	DEV("STUB : %d\n", __LINE__);
	return SCRIPT_ERROR;
}

static void TranslateCall(const v8::FunctionCallbackInfo<v8::Value>& info) {
	auto isolate = info.GetIsolate();
	auto pScriptFunction = static_cast<ScriptFunctionBinding_t*>(v8::Local<v8::External>::Cast(info.Data())->Value());
	if(pScriptFunction->descriptor.arguments.elements)
	DEV("TranslateCall %s\n", pScriptFunction->descriptor.functionName);
}

void JavaScriptVM::RegisterFunction(ScriptFunctionBinding_t* pScriptFunction) {
	v8::HandleScope handleScope(this->isolate);
	v8::Local<v8::Context> context = v8::Local<v8::Context>::New(this->isolate, this->globalContext);
	v8::Context::Scope contextScope(context);

	context->Global()->Set(
		context,
		v8::String::NewFromUtf8(
			this->isolate,
			pScriptFunction->descriptor.functionName
		).ToLocalChecked(),
		v8::FunctionTemplate::New(
			this->isolate,
			TranslateCall,
			v8::External::New(
				this->isolate,
				pScriptFunction
			)
		)->GetFunction(context).ToLocalChecked()
	);
}

bool JavaScriptVM::RegisterClass(ScriptClassDesc_t* pClassDesc) {
	v8::HandleScope handleScope(this->isolate);
	v8::Local<v8::Context> context = v8::Local<v8::Context>::New(this->isolate, this->globalContext);
	v8::Context::Scope contextScope(context);

	auto x = v8::Object::New(this->isolate);

	DEV("RegisterClass : %s\n", pClassDesc->className);
	return false;
}

HSCRIPT JavaScriptVM::RegisterInstance(ScriptClassDesc_t* pDesc, void* pInstance) {
	DEV("RegisterInstance : %s @ %p\n", pDesc->className, pInstance);
	return nullptr;
}

void JavaScriptVM::SetInstanceUniqeId(HSCRIPT hInstance, const char* pszId) {
	DEV("STUB : %d\n", __LINE__);
}

void JavaScriptVM::RemoveInstance(HSCRIPT) {
	DEV("STUB : %d\n", __LINE__);
}

void* JavaScriptVM::GetInstanceValue(HSCRIPT hInstance, ScriptClassDesc_t* pExpectedType) {
	DEV("STUB : %d\n", __LINE__);
	return nullptr;
}

bool JavaScriptVM::GenerateUniqueKey(const char* pszRoot, char* pBuf, int nBufSize) {
	if(strlen(pszRoot) + 18 <= nBufSize) {
		snprintf(pBuf, nBufSize, "%llx_%s", this->serialNumber++, pszRoot);
		return true;
	} else if(nBufSize) {
		*pBuf = 0;
	}
	return false;
}

bool JavaScriptVM::ValueExists(HSCRIPT hScope, const char* pszKey) {
	DEV("STUB : %d\n", __LINE__);
	return false;
}

bool JavaScriptVM::SetValue(HSCRIPT hScope, const char* pszKey, const ScriptVariant_t& value) {
	DEV("STUB : %d\n", __LINE__);
	return false;
}

bool JavaScriptVM::SetValue(HSCRIPT hScope, const char* pszKey, const char* pszValue) {
	DEV("SetValue : %s <- %s\n", pszKey, pszValue);
	return false;
}

void JavaScriptVM::CreateTable(ScriptVariant_t& Table) {
	DEV("STUB : %d\n", __LINE__);
}

int	JavaScriptVM::GetNumTableEntries(HSCRIPT hScope) {
	DEV("STUB : %d\n", __LINE__);
	return 0;
}

int JavaScriptVM::GetKeyValue(HSCRIPT hScope, int nIterator, ScriptVariant_t* pKey, ScriptVariant_t* pValue) {
	DEV("STUB : %d\n", __LINE__);
	return 0;
}

bool JavaScriptVM::GetValue(HSCRIPT hScope, const char* pszKey, ScriptVariant_t* pValue) {
	DEV("STUB : %d\n", __LINE__);
	return false;
}

void JavaScriptVM::ReleaseValue(ScriptVariant_t& value) {
	DEV("STUB : %d\n", __LINE__);
}

bool JavaScriptVM::ClearValue(HSCRIPT hScope, const char* pszKey) {
	DEV("STUB : %d\n", __LINE__);
	return false;
}

void JavaScriptVM::WriteState(CUtlBuffer* pBuffer) {
	DEV("STUB : %d\n", __LINE__);
}

void JavaScriptVM::ReadState(CUtlBuffer* pBuffer) {
	DEV("STUB : %d\n", __LINE__);
}

void JavaScriptVM::RemoveOrphanInstances() {
	DEV("STUB : %d\n", __LINE__);
}

void JavaScriptVM::DumpState() {
	DEV("STUB : %d\n", __LINE__);
}

void JavaScriptVM::SetOutputCallback(ScriptOutputFunc_t pFunc) {
	DEV("STUB : %d\n", __LINE__);
}

void JavaScriptVM::SetErrorCallback(ScriptErrorFunc_t pFunc) {
	DEV("STUB : %d\n", __LINE__);
}

bool JavaScriptVM::RaiseException(const char* pszExceptionText) {
	DEV("STUB : %d\n", __LINE__);
	return false;
}


IScriptVM* ScriptCreateJavaScriptVM() {
	return new JavaScriptVM;
}

void ScriptDestroyJavaScriptVM(IScriptVM* vm) {
	delete static_cast<JavaScriptVM*>(vm);
}