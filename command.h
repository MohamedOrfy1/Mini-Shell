#ifndef COMMAND_H
#define COMMAND_H

// Command Data Structure
struct SimpleCommand {
    // Available space for arguments currently preallocated
    int _numberOfAvailableArguments;

    // Number of arguments
    int _numberOfArguments;
    char **_arguments;

    SimpleCommand();
    void insertArgument(char *argument);
};

struct Command {
    int _numberOfAvailableSimpleCommands;
    int _numberOfSimpleCommands;
    SimpleCommand **_simpleCommands;
    char *_outFile;
    char *_inputFile;
    char *_errFile;
    int _greaterFlag;
    int _background;
    int _append;
    int _appendBack; // Changed from _appback to _appendBack for consistency

    void prompt();
    void print();
    void execute();
    void clear();

    Command();
    void insertSimpleCommand(SimpleCommand *simpleCommand);

    static Command _currentCommand;
    static SimpleCommand *_currentSimpleCommand;
};

#endif
