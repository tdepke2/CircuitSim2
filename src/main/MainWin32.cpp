#ifdef _WIN32

#include <cstdio>
#include <iostream>
#include <windows.h>

extern int main(int argc, char* argv[]);

/**
 * On Linux, running the main application from a console uses the console for
 * stdout/stdin, while running it without a console directs stdout/stdin to the
 * null device. I don't know why this is so hard to replicate in Windows, it
 * should be this way by default!
 * 
 * Below is some Windows-specific code to accomplish redirecting output to the
 * console. It has some issues though and probably isn't very future proof, so
 * it is only here for reference.
 * 
 * This code has been adapted from:
 * https://stackoverflow.com/a/55875595
 */

bool redirectConsoleIO() {
    bool success = true;
    FILE* fp = nullptr;

    // Redirect STDIN if the console has an input handle.
    if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE) {
        if (freopen_s(&fp, "CONIN$", "r", stdin) != 0) {
            success = false;
        } else {
            setvbuf(stdin, nullptr, _IONBF, 0);
        }
    }

    // Redirect STDOUT if the console has an output handle.
    if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE) {
        if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0) {
            success = false;
        } else {
            setvbuf(stdout, nullptr, _IONBF, 0);
        }
    }

    // Redirect STDERR if the console has an error handle.
    if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE) {
        if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0) {
            success = false;
        } else {
            setvbuf(stderr, nullptr, _IONBF, 0);
        }
    }

    // Make C++ standard streams point to console as well.
    std::ios::sync_with_stdio(true);

    // Clear the error state for each of the C++ standard streams.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();

    return success;
}

bool releaseConsole() {
    bool success = true;
    FILE* fp = nullptr;

    // Just to be safe, redirect standard IO to NULL before releasing.

    // Redirect STDIN to NULL.
    if (freopen_s(&fp, "NUL:", "r", stdin) != 0) {
        success = false;
    } else {
        setvbuf(stdin, nullptr, _IONBF, 0);
    }

    // Redirect STDOUT to NULL.
    if (freopen_s(&fp, "NUL:", "w", stdout) != 0) {
        success = false;
    } else {
        setvbuf(stdout, nullptr, _IONBF, 0);
    }

    // Redirect STDERR to NULL.
    if (freopen_s(&fp, "NUL:", "w", stderr) != 0) {
        success = false;
    } else {
        setvbuf(stderr, nullptr, _IONBF, 0);
    }

    // Detach from console.
    if (!FreeConsole()) {
        success = false;
    }

    return success;
}

void adjustConsoleBuffer(int16_t minLength) {
    // Set the screen buffer to be big enough to scroll some text.
    CONSOLE_SCREEN_BUFFER_INFO conInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
    if (conInfo.dwSize.Y < minLength) {
        conInfo.dwSize.Y = minLength;
    }
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);
}

bool createNewConsole(int16_t minLength) {
    bool success = false;

    // Release any current console and redirect IO to NULL.
    releaseConsole();

    // Attempt to create new console.
    if (AllocConsole()) {
        adjustConsoleBuffer(minLength);
        success = redirectConsoleIO();
    }

    return success;
}

bool attachParentConsole(int16_t minLength) {
    bool success = false;

    // Release any current console and redirect IO to NULL.
    releaseConsole();

    // Attempt to attach to parent process's console.
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        adjustConsoleBuffer(minLength);
        success = redirectConsoleIO();
    }

    return success;
}

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nCmdShow*/) {
    /*if (attachParentConsole(1024)) {
        // Call main() here.
        releaseConsole();
    }
    return 0;*/

    // Ignore warnings about calling main() from within another function.
    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmain"
    #endif
    return main(__argc, __argv);
    #if defined(__GNUC__) || defined(__clang__)
        #pragma GCC diagnostic pop
    #endif
}

#endif
