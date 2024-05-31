/**
 * To enable debug messages in the GUI, pass "-DCS2_GUI_DEBUG=ON" to CMake
 * during the configuration step, then run the build again.
 */
#ifdef CS2_GUI_DEBUG
    #include <iostream>
    #define GUI_DEBUG std::cout
#else
    #include <ostream>

    static std::ostream nullStream(nullptr);
    #define GUI_DEBUG if (true) {;} else nullStream
#endif
