#include "transaq_loader.h"
#include <windows.h>

namespace transaq {
namespace loader {

void* load_library(std::string const& path)
{
    HMODULE module = LoadLibrary(path.c_str());
    if (!module)
    {
        throw std::runtime_error("library not found");
    }
    return static_cast<void*>(module);
}

void unload_library(void * library)
{
    // do not unload dll explicitly, since it hangs
    // FreeLibrary(static_cast<HMODULE>(library));
}

void* load_function(void * library, std::string const& name)
{
    void * address = GetProcAddress(static_cast<HMODULE>(library), name.c_str());
    if (!address)
    {
        throw std::runtime_error("function not found");
    }
    return address;
}

}
}
