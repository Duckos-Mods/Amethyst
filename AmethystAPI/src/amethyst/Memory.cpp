#include "amethyst/Memory.h"
#include <libhat/Scanner.hpp>
#include <mutex>
#include <thread>
#include <optional>

uintptr_t GetMinecraftBaseAddress()
{
    static uintptr_t mc = reinterpret_cast<uintptr_t>(GetModuleHandleA("Minecraft.Windows.exe"));
    return mc;
}

unsigned long GetMinecraftSize()
{
    HMODULE base = GetModuleHandleA("Minecraft.Windows.exe");
    if (base == nullptr) return 0;

    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), base, &moduleInfo, sizeof(MODULEINFO))) return 0;

    return moduleInfo.SizeOfImage;
}

uintptr_t SlideAddress(uintptr_t offset)
{
    return GetMinecraftBaseAddress() + offset;
}

std::optional<uintptr_t> SigScanSafe(std::string_view signature)
{
    const auto parsed = hat::parse_signature(signature);
    if (!parsed.has_value()) {
        Log::Error("Invalid signature! {:s}", signature);
        throw std::exception();
    }

    const auto begin = reinterpret_cast<std::byte*>(GetMinecraftBaseAddress());
    const auto end = begin + GetMinecraftSize();
    const auto result = hat::find_pattern(begin, end, parsed.value());

    if (!result.has_result()) return std::nullopt;
    return reinterpret_cast<uintptr_t>(result.get());
}

uintptr_t SigScan(std::string_view signature) {
    auto result = SigScanSafe(signature);

    if (!result.has_value()) {
        std::string errorMessage = fmt::format("Failed to find signature \"{:s}\"", signature);
        throw std::exception(errorMessage.c_str());
    }

    return result.value();
}

size_t FindOffsetOfPointer(void* _base, void* _pointer, size_t maxSearchSize)
{
    uintptr_t base = reinterpret_cast<uintptr_t>(_base);
    uintptr_t pointer = reinterpret_cast<uintptr_t>(_pointer);

    for (size_t offset = 0; offset < maxSearchSize; offset++) {
        uintptr_t ptrAtOffset = *reinterpret_cast<uintptr_t*>(base + offset);
        if (ptrAtOffset == pointer) return offset;
    }

    return SIZE_MAX;
}

bool UnprotectMemory(void* address, size_t size, DWORD* oldProtection)
{
    return ProtectMemory(address, size, PAGE_EXECUTE_READWRITE, oldProtection);
}

bool ProtectMemory(void* address, size_t size, DWORD protectionData, DWORD* oldProtection)
{
    DWORD oldProtect;
    if (oldProtection != nullptr) oldProtect = *oldProtection;
	else oldProtect = DWORD();
    if (!VirtualProtect(address, size, protectionData, &oldProtect)) {
		Log::Error("Failed to reprotect memory at 0x{:X}! Error: {}", reinterpret_cast<uintptr_t>(address), GetLastError());
		return false;
	}

	if (oldProtection != nullptr) *oldProtection = oldProtect;
	return true;
}

