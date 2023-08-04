#pragma once

#include "singleton.hpp"

#include <libplatform/libplatform.h>

class Main : public Singleton<Main> {
public:
	void Initialize();
	void Shutdown();
	const char* Description();
	void Update();

private:
	std::unique_ptr<v8::Platform> platform;
};
