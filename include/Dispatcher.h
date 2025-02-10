#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <string>
#include <vector>
#include "Command.h"

class Dispatcher {
public:
	Dispatcher();

	// Register a top‑level command; throws if a duplicate name or alias is detected.
	void registerCommand(const Command& cmd);

	// Parse an input string, validate arguments, and execute the matching command.
	// Throws std::runtime_error on errors.
	bool dispatch(const std::string& input);

	// Print global help for all registered commands.
	void printGlobalHelp() const;

private:
	std::vector<Command> commands;

	std::vector<std::string> tokenize(const std::string& input);

	const Command* matchCommand(const std::vector<std::string>& tokens, size_t& index);

	void parseArguments(const std::vector<std::string>& tokens, size_t index, std::vector<Argument>& outArgs);

	Value parseValue(const std::string& token);

	Value parseList(const std::string& token);
};

#endif
