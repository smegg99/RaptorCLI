// include/executable_command.h
#ifndef EXECUTABLE_COMMAND_H
#define EXECUTABLE_COMMAND_H

#include <string>
#include <vector>
#include <initializer_list>
#include "command.h"
#include "argument.h"
#include "value.h"

// Class representing a command with pre-defined argument values.
class ExecutableCommand {
public:
	ExecutableCommand() = default;
	ExecutableCommand(const Command& baseCmd, const std::vector<Argument>& presetArgs);
	ExecutableCommand(const Command& baseCmd, std::initializer_list<std::pair<std::string, Value>> presetArgsList);

	// Executes the command by calling the base command's callback with the preset arguments.
	bool execute() const;

	// Executes the command by calling the base command's callback with the provided arguments.
	bool executeWithArgs(std::initializer_list<std::pair<std::string, Value>> argsList) const;

	// Returns a string representation of the command in the form:
	// "commandName -arg1 value1 -arg2 "value2""
	std::string toString() const;

	// Returns a string representation of the command with the provided arguments.
	std::string toStringWithArgs(std::initializer_list<std::pair<std::string, Value>> argsList) const;

	// Prints to the base command's output the command with the provided arguments.
	void toOutputWithArgs(std::initializer_list<std::pair<std::string, Value>> argsList) const;

	const Command& getBaseCommand() const;

	std::vector<Argument> getPresetArgs() const;
	void setArgs(const std::vector<Argument>& presetArgs);

private:
	Command baseCommand;
	std::vector<Argument> presetArgs;
};

// Class representing a sequence of executable commands.
class CommandSequence {
public:
	// Add an executable command to the sequence.
	void addCommand(const ExecutableCommand& cmd);

	// Execute all commands in the sequence one after another.
	// Returns true only if every command executed successfully.
	bool execute() const;

	// Returns a string representing the entire sequence.
	std::string toString() const;

private:
	std::vector<ExecutableCommand> commands;
};

#endif
