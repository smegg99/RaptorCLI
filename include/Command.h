#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include "Argument.h"

class Command;
typedef void (*CommandCallback)(const Command&);

class Command {
public:
	std::string name;
	std::string description;
	std::vector<std::string> aliases;    // Additional names for the command
	std::vector<Command> subcommands;    // Optional child commands
	std::vector<Argument> arguments;     // Parsed arguments after dispatch
	std::vector<ArgSpec> argSpecs;       // Declared expected arguments

	CommandCallback callback;

	Command();
	Command(const std::string& cmdName, const std::string& desc = "");

	// Add a subcommand (throws if a duplicate name/alias exists).
	void addSubcommand(const Command& cmd);

	// Add an alias (throws if the alias equals the command’s name or duplicates an existing alias).
	void addAlias(const std::string& alias);

	// Add an expected argument specification (throws if duplicate).
	void addArgSpec(const ArgSpec& spec);

	// Print usage information for this command and recursively for its subcommands.
	void printUsage(const std::string& prefix = "") const;
};

#endif
