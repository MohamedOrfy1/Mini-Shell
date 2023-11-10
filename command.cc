/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <csignal>
#include <ctime>

#include <time.h>
#include <iostream>
#include <string>
#include <iostream>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#include "command.h"

int child_pid;

SimpleCommand::SimpleCommand() {
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument) {
    if (_numberOfAvailableArguments == _numberOfArguments + 1) {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments, _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command() {
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _append = 0;
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand) {
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands) {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands, _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear() {
    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++) {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile) {
        free(_outFile);
    }

    if (_inputFile) {
        free(_inputFile);
    }

    if (_errFile) {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _append = 0;
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++) {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
        printf("\n");
    }

    printf("\n\n");
    printf("  Output       Input        Error        Background\n");
    printf("  ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default",
           _background ? "YES" : "NO");
    printf("\n\n");
}

void PrintDebugging(int _numberOfSimpleCommands, SimpleCommand **_simpleCommands) {
    char *arg;
    int argIndex;
    printf("---number of simple commands %d---\n", _numberOfSimpleCommands);
    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        printf("%d th command has %d argument\n", i, _simpleCommands[i]->_numberOfArguments);
        argIndex = 0;
        arg = _simpleCommands[i]->_arguments[argIndex];
        while (arg != NULL) {
            printf("argument-> %s\n", arg);
            argIndex++;
            arg = _simpleCommands[i]->_arguments[argIndex];
            printf("END OF WHILE-LOOP");
        }
        printf("END OF FOR-LOOP");
    }
    printf("---END OF DEBUGGING PRINT---");
}

void handler(int sig) {
    int pid = wait(NULL);
    int status;
    time_t currentTime = time(NULL);
    struct tm currentDateTime = *localtime(&currentTime);
    if (pid != -1) {
        FILE *logFile = fopen("logfile", "a");

        fprintf(logFile, "%02d-%02d-%02d %02d/%02d/%d\n", currentDateTime.tm_hour, currentDateTime.tm_min, currentDateTime.tm_sec, currentDateTime.tm_mday, currentDateTime.tm_mon + 1,
                currentDateTime.tm_year + 1900);
        fprintf(logFile, "------------------------------------------------------\n");
        fprintf(logFile, "%d child process terminated\n", pid);
        fprintf(logFile, "------------------------------------------------------\n");
        signal(sig, handler);

        fclose(logFile);
    }
}

void Command::execute() {
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, handler);

    if (_numberOfSimpleCommands == 1 && _simpleCommands[0]->_numberOfArguments == 1 &&
        strcmp(_simpleCommands[0]->_arguments[0], "exit") == 0) {
        exit(0);
    }

    if (_numberOfSimpleCommands == 0) {
        prompt();
        return;
    }

    if (_numberOfSimpleCommands == 1 && _simpleCommands[0]->_numberOfArguments <= 2 &&
        strcmp(_simpleCommands[0]->_arguments[0], "cd") == 0) {
        printf("cd intercepted\n");
        if (_simpleCommands[0]->_numberOfArguments == 1) {
            int ch = chdir(getenv("HOME"));
        } else if (chdir(_simpleCommands[0]->_arguments[1]) == -1)
            perror("directory doesn't exist\n");

        print();
        clear();
        prompt();
        return;
    }

    print();

    int defaultInput = dup(0);
    int defaultOutput = dup(1);
    int defaultError = dup(2);
    int inputFileDescriptor, outputFileDescriptor, errorFileDescriptor;

    if (_inputFile) {
        inputFileDescriptor = open(_inputFile, O_RDONLY);
        if (inputFileDescriptor < 0) {
            perror("error : create input file");
            exit(2);
        } else
            dup2(inputFileDescriptor, 0);

    } else
        inputFileDescriptor = dup(defaultInput);

    if (_errFile) {
        errorFileDescriptor = open(_errFile, O_TRUNC | O_CREAT | O_WRONLY, 0666);
        if (errorFileDescriptor < 0) {
            perror("error : create error file");
            exit(2);
        } else
            dup2(errorFileDescriptor, 2);
    } else
        errorFileDescriptor = defaultError;

    int pid, i;
    for (i = 0; i < _numberOfSimpleCommands; i++) {
        dup2(inputFileDescriptor, 0);
        close(inputFileDescriptor);
        if (i == _numberOfSimpleCommands - 1) {
            if (_outFile) {
                if (_append == 1) {
                    outputFileDescriptor = open(_outFile, O_APPEND | O_WRONLY | O_CREAT, 0666);
                    if (outputFileDescriptor < 0) {
                        perror("error : bad access to output file or non-existent");
                        exit(2);
                    } else {
                        dup2(outputFileDescriptor, 1);
                        if (_appendBack == 1)
                            dup2(outputFileDescriptor, 2);
                    }
                } else {
                    outputFileDescriptor = open(_outFile, O_TRUNC | O_CREAT | O_WRONLY, 0666);
                    if (outputFileDescriptor < 0) {
                        perror("error : bad access to output file or non-existent");
                        exit(2);
                    } else {
                        dup2(outputFileDescriptor, 1);
                        if (_appendBack == 1)
                            dup2(outputFileDescriptor, 2);
                    }
                }
            } else {
                outputFileDescriptor = dup(defaultOutput);
            }
        } else {
            int pipeFileDescriptors[2];
            if (pipe(pipeFileDescriptors) == -1) {
                perror("command : pipe");
                exit(2);
            }
            inputFileDescriptor = pipeFileDescriptors[0];
            outputFileDescriptor = pipeFileDescriptors[1];
        }

        dup2(outputFileDescriptor, 1);
        close(outputFileDescriptor);
        pid = fork();

        if (pid == -1) {
            perror("command: fork\n");
            exit(2);
        }

        if (pid == 0) {
            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            perror("piping error :");
            exit(1);
        }
        raise(SIGCHLD);

        if (i == _numberOfSimpleCommands - 1 && !_background) {
            waitpid(pid, 0, 0);
        }
    }

    dup2(defaultInput, 0);
    dup2(defaultOutput, 1);
    dup2(defaultError, 2);

    close(inputFileDescriptor);
    close(outputFileDescriptor);
    close(errorFileDescriptor);

    clear();
    prompt();
}

void Command::prompt() {
    signal(SIGINT, SIG_IGN);
    char currentWorkingDirectory[256];
    getcwd(currentWorkingDirectory, 256);
    printf("myshell>%s ", currentWorkingDirectory);
    fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int yyparse(void);

int main() {
    Command::_currentCommand.prompt();
    yyparse();
    return 0;
}
