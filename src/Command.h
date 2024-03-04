#pragma once

class Command {
public:
    Command() = default;
    virtual ~Command() = default;
    Command(const Command& rhs) = delete;
    Command& operator=(const Command& rhs) = delete;

    virtual void execute() = 0;
    virtual void undo() = 0;
};
