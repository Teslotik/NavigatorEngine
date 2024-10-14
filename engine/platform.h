#ifndef PLATFORM_H
#define PLATFORM_H

// https://github.com/boostorg/predef/tree/master
#if defined(_WIN32) || defined(_WIN64) ||  defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#   define WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
#   define LINUX
#else
#   error "Unsupported platform"
#endif

#include <string>

#include "Loader.h"

#ifdef WINDOWS
#define NOMINMAX
#include <windows.h>
#endif

namespace engine {

#ifdef LINUX
class Platform {
public:
    void openUrl(std::string url) {
        system(("xdg-open " + url).c_str());
    }

    // Loader createLoader(std::string) {

    // }
};
#endif

#ifdef WINDOWS
class Platform {
public:
    void openUrl(std::string url) {
        system(("start " + url).c_str());
    }
};
#endif

} // namespace engine

#endif