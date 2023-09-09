#include "patch.hpp"
#include <vector>
#include <cstring>
#ifdef _WIN32
#include <Windows.h>
#define PROTECT(target, size, protect) \
	{ \
		DWORD dummy; \
		VirtualProtect(target, size, protect, &dummy); \
	}
#define PROTECT_XWR PAGE_EXECUTE_READWRITE
#define PROTECT_XR PAGE_EXECUTE_READ
#else
#include <sys/mman.h>
// TODO: cache a value from sysconf or whatever it was to get the actual pgsize
#define PROTECT_PAGE_SIZE 0x1000
#define PROTECT(target, size, protect) \
	{ \
		auto base = reinterpret_cast<uintptr_t>(target); \
		auto alignedStart = base & ~(PROTECT_PAGE_SIZE - 1); \
		auto alignedEnd = (base + size) & ~(PROTECT_PAGE_SIZE - 1); \
		auto alignedSize = alignedEnd - alignedStart + PROTECT_PAGE_SIZE; \
		mprotect(reinterpret_cast<void*>(alignedStart), alignedSize, protect); \
	}
#define PROTECT_XWR PROT_EXEC | PROT_WRITE | PROT_READ 
#define PROTECT_XR PROT_EXEC | PROT_READ
#endif


class BytePatch : public Patch {
public:
	BytePatch(void* target, std::vector<uint8_t> bytes) : target(target) {
		this->size = bytes.size();
		this->original.reserve(this->size);
		std::memcpy(this->original.data(), this->target, this->size);
		PROTECT(this->target, this->size, PROTECT_XWR);
		std::memcpy(this->target, bytes.data(), this->size);
		PROTECT(this->target, this->size, PROTECT_XR);
	}

	template<typename T> BytePatch(void* target, T value) : target(target) {
		this->size = sizeof(T);
		this->original.reserve(this->size);
		std::memcpy(this->original.data(), this->target, this->size);
		PROTECT(this->target, this->size, PROTECT_XWR);
		std::memcpy(this->target, &value, this->size);
		PROTECT(this->target, this->size, PROTECT_XR);
	}

	~BytePatch() {
		PROTECT(this->target, this->size, PROTECT_XWR);
		std::memcpy(this->target, this->original.data(), this->size);
		PROTECT(this->target, this->size, PROTECT_XR);
	}

private:
	size_t size;
	void* target;
	std::vector<uint8_t> original;
};
