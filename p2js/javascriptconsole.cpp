#include "javascriptconsole.hpp"

void JavaScriptConsole::Debug(const v8::debug::ConsoleCallArguments& args, const v8::debug::ConsoleContext& context) {
	DEV("%s", this->TranslateArgs(args).c_str());
}

void JavaScriptConsole::Log(const v8::debug::ConsoleCallArguments& args, const v8::debug::ConsoleContext& context) {
	INFO("%s", this->TranslateArgs(args).c_str());
}

void JavaScriptConsole::Warn(const v8::debug::ConsoleCallArguments& args, const v8::debug::ConsoleContext& context) {
	WARNING("%s", this->TranslateArgs(args).c_str());
}

void JavaScriptConsole::Error(const v8::debug::ConsoleCallArguments& args, const v8::debug::ConsoleContext& context) {
	ERROR("%s", this->TranslateArgs(args).c_str());
}

std::string JavaScriptConsole::TranslateArgs(const v8::debug::ConsoleCallArguments& args) {
	v8::HandleScope handleScope(this->isolate);

	std::stringstream stream;

	for(int i = 0; i < args.Length(); i++) {
		if(i > 0) stream << " ";

		auto arg = args[i];
		if(arg->IsSymbol()) arg = v8::Local<v8::Symbol>::Cast(arg)->Description(this->isolate);

		v8::Local<v8::String> string;
		if(!arg->ToString(this->isolate->GetCurrentContext()).ToLocal(&string)) continue;

		v8::String::Utf8Value stringValue(this->isolate, string);
		stream << *stringValue;
	}

	stream << std::endl;

	return stream.str();
}