// include/dispatcher.h
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <string>
#include <vector>
#include "command.h"
#include "clioutput.h"

class Dispatcher {
public:
	Dispatcher();

	CLIOutput* output;

	// Register an output interface for printing CLI messages.
	void registerOutput(CLIOutput* output);

	// Register a top‑level command; returns true if successful, false if an error occurred.
	bool registerCommand(const Command& cmd);

	// Parse an input string, validate arguments, and execute the matching command.
	// Returns true on success, false on error.
	bool dispatch(const std::string& input);

	// Print global help for all registered commands.
	void printGlobalHelp() const;

	// Get the current output interface.
	CLIOutput* getOutput();
private:
	std::vector<Command> commands;

	std::vector<std::string> tokenize(const std::string& input);

	const Command* matchCommand(const std::vector<std::string>& tokens, size_t& index);

	// Returns false on error.
	bool parseArguments(const std::vector<std::string>& tokens, size_t index, std::vector<Argument>& outArgs);

	Value parseValue(const std::string& token);

	Value parseList(const std::string& token);
};

#endif
