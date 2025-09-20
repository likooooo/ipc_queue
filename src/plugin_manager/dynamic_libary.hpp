#pragma once
#include <string>
#include <stdexcept>

#if defined(_WIN32)
  #include <windows.h>
  using LibHandle = HMODULE;
#else
  #include <dlfcn.h>
  using LibHandle = void*;
#endif

class dynamic_library {
public:
    dynamic_library() : handle(nullptr) {}
    ~dynamic_library() { close(); }

    void open(const std::string& path) {
        close();
#if defined(_WIN32)
        handle = LoadLibraryA(path.c_str());
#else
        handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
        if (!handle) {
#if defined(_WIN32)
            throw std::runtime_error("LoadLibrary failed: " + path);
#else
            const char* err = dlerror();
            throw std::runtime_error(std::string("dlopen failed: ") + (err ? err : "unknown") + " path=" + path);
#endif
        }
    }

    void close() {
        if (!handle) return;
#if defined(_WIN32)
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        handle = nullptr;
    }

    void* symbol(const std::string& name) {
        if (!handle) return nullptr;
#if defined(_WIN32)
        return reinterpret_cast<void*>(GetProcAddress(handle, name.c_str()));
#else
        return dlsym(handle, name.c_str());
#endif
    }

private:
    LibHandle handle;
};