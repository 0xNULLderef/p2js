#include "main.hpp"

#include "logger.hpp"
#include "hooks.hpp"
#include <v8-initialization.h>
#include <libplatform/libplatform.h>

#ifndef _WIN32
#include <unistd.h>
#include <linux/limits.h>
#endif

void Main::Initialize() {
#ifdef _WIN32
	char exe[MAX_PATH];
	GetModuleFileNameA(GetModuleHandleA(nullptr), exe, sizeof(exe));
#else
	char exe[PATH_MAX];
	readlink("/proc/self/exe", exe, sizeof(exe));
#endif
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
