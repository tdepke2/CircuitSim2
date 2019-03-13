#include "Simulator.h"

using namespace std;

int main() {
    return Simulator::start();
}

/*
Notes:
  - Gonna need additional tri-state (dashed color) and conflict (red color) wire states. These will save as unpowered wires however.
  - Implement the original wire algorithm first for backwards compatibility.
*/