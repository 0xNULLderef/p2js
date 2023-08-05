#pragma once

#include "logger.hpp"
#undef min
#undef max
#include <src/debug/debug-interface.h>

class JavaScriptConsole : public v8::debug::ConsoleDelegate {
public:
	JavaScriptConsole(v8::Isolate* isolate) {
		this->isolate = isolate;
	}

	void Debug(const v8::debug::ConsoleCallArguments& args, const v8::debug::ConsoleContext& context);
	void Log(const v8::debug::ConsoleCallArguments& args, const v8::debug::ConsoleContext& context);
	void Warn(const v8::debug::ConsoleCallArguments& args, const v8::debug::ConsoleContext& context);
	void Error(const v8::debug::ConsoleCallArguments& args, const v8::debug::ConsoleContext& context);

	std::string TranslateArgs(const v8::debug::ConsoleCallArguments& args);

private:
	v8::Isolate* isolate;
};
