#include "main.hpp"

#include "logger.hpp"
#include "hooks.hpp"
#include <v8-initialization.h>
#include <libplatform/libplatform.h>

void Main::Initialize() {
	auto exe = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Portal 2\\portal2.exe";
	v8::V8::InitializeICUDefaultLocation(exe);
	v8::V8::InitializeExternalStartupData(exe);
	this->platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(this->platform.get());
	v8::V8::Initialize();

	Hooks::Instance()->Initialize();

	INFO("p2js loaded :3\n");
}

void Main::Shutdown() {
	Hooks::Instance()->Shutdown();

	v8::V8::Dispose();
	v8::V8::DisposePlatform();

	INFO("Yoinking >///<\n");
}

const char* Main::Description() {
	return "Horrors beyond human comprehension :3";
}

void Main::Update() {

}
