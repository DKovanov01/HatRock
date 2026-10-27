#pragma once
#define _CRTDBG_MAP_ALLOC // works with the 'look for memory leaks' thing below
#include <iostream> // Or any other standard library headers
#include <stdlib.h> // stdlib.h is a C standard library header that provides functions for memory allocation, process control, conversions, and other utilities.
#include <crtdbg.h> // crtdbg.h is a C runtime library header that provides functions for debugging memory allocation and detecting memory leaks in C and C++ programs.
#include "Debug.h"
#include <string>

#include "MemoryMonitor.h"

#include "MMath.h"
using namespace MATH;

int main() {
    #pragma warning(disable : 4996)  // Disable warning C4996: 'function': was declared deprecated. This warning is generated when using functions that have been marked as deprecated in the C++ standard library. Scott added it for reasons.
    /*if (const char* env_p = std::getenv("PATH")) {
        std::cout << "Your PATH is: " << env_p << '\n';
    }*/

    static_assert(sizeof(int*) == 8, "Vulkan no longer supports x86 builds");  //means "close the program right now" if there's an error, won't even let me build
    // basically can't change to x86

    std::cout << "HatRock Engine starting...\n";
    std::cout << "Press Enter to run..." << std::endl;

    std::cin.get();

    Debug::DebugInit("Vulkan Graphics Engine Log");
    Debug::Info("HatRock Engine starting through GameSceneManager", __FILE__, __LINE__);
    /*SceneManager* gsm = new SceneManager();
    if (gsm->Initialize("Vulkan Graphics Engine", 1280, 720) == true) {
        gsm->Run();
    }*/

    //delete gsm;
    writeMemoryLog();

    return 0;
}
