#pragma once

#include <chrono>
#include <string>

class Command {
public:
    Command();
    virtual ~Command() = default;
    Command(const Command& rhs) = delete;
    Command& operator=(const Command& rhs) = delete;

    std::chrono::milliseconds getTimeSinceLastExecute() const;

    // Gets a string describing the operation the command will perform.
    virtual std::string getMessage() const = 0;
    // Determines if the command can be added to and executed again.
    virtual bool isGroupingAllowed() const = 0;
    virtual void execute();
    virtual void undo() = 0;

private:
    using Clock = std::chrono::steady_clock;

    Clock::time_point lastExecuteTime_;
};
