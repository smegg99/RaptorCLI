// include/command.h
#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>
#include "argument.h"
#include "clioutput.h"

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

	// Add a subcommand (returns true if added successfully, false on error).
	bool addSubcommand(const Command& cmd);

	// Add an alias (returns true if added successfully, false on error).
	bool addAlias(const std::string& alias);

	// Add an expected argument specification (returns true if added successfully, false on error).
	bool addArgSpec(const ArgSpec& spec);

	// Print usage information for this command and recursively for its subcommands.
	void printUsage(const std::string& prefix = "", CLIOutput* output = nullptr) const;

	void registerOutput(CLIOutput* out);

	CLIOutput* getOutput() const;
private:
	CLIOutput* output;
};

#endif
