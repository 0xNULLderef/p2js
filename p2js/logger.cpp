#include "logger.hpp"

#ifndef _WIN32
#include <dlfcn.h>
#endif

Logger::Logger() {
#ifdef _WIN32
	auto tier0 = GetModuleHandleA("tier0");
	if(tier0 != nullptr) {
		this->ColorMsg = reinterpret_cast<ColorMsg_t>(GetProcAddress(tier0, "?ConColorMsg@@YAXABVColor@@PBDZZ"));
	}
#else
	auto tier0 = dlopen("tier0.so", RTLD_NOLOAD | RTLD_NOW);
	if(tier0 != nullptr) {
		this->ColorMsg = reinterpret_cast<ColorMsg_t>(dlsym(tier0, "?ConColorMsg@@YAXABVColor@@PBDZZ"));
	}
#endif

#ifdef DEBUG
#ifdef _WIN32
	AllocConsole();
	FILE* dummy = nullptr;
	freopen_s(&dummy, "CONOUT$", "w", stdout);

	CONSOLE_SCREEN_BUFFER_INFOEX consoleInfo = { };
	consoleInfo.cbSize = sizeof(consoleInfo);

	const auto consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfoEx(consoleHandle, &consoleInfo);
	consoleInfo.ColorTable[7] = LOG_COLOR.ToBGR();
	SetConsoleScreenBufferInfoEx(consoleHandle, &consoleInfo);
#endif
#endif
}

Logger::~Logger() {
	// don't log to source console on destroy, shit would go south quick
	this->ColorMsg = nullptr;
}
