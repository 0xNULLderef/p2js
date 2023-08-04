#include "patch.hpp"
#include <vector>
#include <Windows.h>

class BytePatch : public Patch {
public:
	BytePatch(void* target, std::vector<uint8_t> bytes) : target(target) {
		this->size = bytes.size();
		this->original.reserve(this->size);
		std::memcpy(this->original.data(), this->target, this->size);
		DWORD protect;
		VirtualProtect(this->target, this->size, PAGE_EXECUTE_READWRITE, &protect);
		std::memcpy(this->target, bytes.data(), this->size);
		DWORD dummy;
		VirtualProtect(this->target, this->size, protect, &dummy);
	}

	~BytePatch() {
		DWORD protect;
		VirtualProtect(this->target, this->size, PAGE_EXECUTE_READWRITE, &protect);
		std::memcpy(this->target, this->original.data(), this->size);
		DWORD dummy;
		VirtualProtect(this->target, this->size, protect, &dummy);
	}

private:
	size_t size;
	void* target;
	std::vector<uint8_t> original;
};
