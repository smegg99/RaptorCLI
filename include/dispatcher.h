// include/dispatcher.h
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <string>
#include <vector>
#include "command.h"
#include "clioutput.h"

// The Dispatcher class is responsible for tokenizing, parsing, and executing CLI commands.
class Dispatcher {
public:
	Dispatcher();

	CLIOutput* output;

	// Register an output interface for printing CLI messages.
	void registerOutput(CLIOutput* output);

	// Register a top‑level command; returns true if successful, false if an error occurred.
	bool registerCommand(const Command& cmd);

	// Parse an input string, validate arguments, and execute the matching command.
	// The input may contain multiple commands separated by ';'. Returns true on success, false on error.
	bool dispatch(const std::string& input);

	// Print global help for all registered commands.
	void printGlobalHelp() const;

	// Get the current output interface.
	CLIOutput* getOutput();
private:
	std::vector<Command> commands;

	// Tokenize an input string into individual tokens.
	std::vector<std::string> tokenize(const std::string& input);

	// Match the command from tokens and update the token index.
	const Command* matchCommand(const std::vector<std::string>& tokens, size_t& index);

	// Parse the arguments from tokens starting at index; returns false on error.
	bool parseArguments(const std::vector<std::string>& tokens, size_t index, std::vector<Argument>& outArgs);

	// Parse a token into a Value.
	Value parseValue(const std::string& token);

	// Parse a token representing a list into a Value of type list.
	Value parseList(const std::string& token);

	// Dispatch a single command string (after cleanup).
	bool dispatchSingleCommand(const std::string& command);
};

#endif
