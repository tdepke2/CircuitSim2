#include <Command.h>

Command::Command() :
    lastExecuteTime_() {
}

std::chrono::milliseconds Command::getTimeSinceLastExecute() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - lastExecuteTime_);
}

void Command::execute() {
    lastExecuteTime_ = Clock::now();
}
