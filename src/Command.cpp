#include "Command.h"
#include "CLIOutput.h"
#include <stdexcept>

Command::Command() : name(""), callback(0) {}

Command::Command(const std::string& cmdName, const std::string& desc)
	: name(cmdName), description(desc), callback(0) {
}

void Command::addSubcommand(const Command& cmd) {
	for (size_t i = 0; i < subcommands.size(); i++) {
		if (subcommands[i].name == cmd.name) {
			throw std::runtime_error("Duplicate subcommand name: " + cmd.name);
		}
		for (size_t j = 0; j < cmd.aliases.size(); j++) {
			if (subcommands[i].name == cmd.aliases[j]) {
				throw std::runtime_error("Duplicate subcommand alias: " + cmd.aliases[j]);
			}
			for (size_t k = 0; k < subcommands[i].aliases.size(); k++) {
				if (subcommands[i].aliases[k] == cmd.aliases[j]) {
					throw std::runtime_error("Duplicate subcommand alias: " + cmd.aliases[j]);
				}
			}
		}
	}
	subcommands.push_back(cmd);
}

void Command::addAlias(const std::string& alias) {
	if (alias == name) {
		throw std::runtime_error("Alias cannot be the same as the command name: " + alias);
	}
	for (size_t i = 0; i < aliases.size(); i++) {
		if (aliases[i] == alias) {
			throw std::runtime_error("Duplicate alias: " + alias);
		}
	}
	aliases.push_back(alias);
}

void Command::addArgSpec(const ArgSpec& spec) {
	for (size_t i = 0; i < argSpecs.size(); i++) {
		if (argSpecs[i].name == spec.name) {
			throw std::runtime_error("Duplicate argument specification: " + spec.name);
		}
	}
	argSpecs.push_back(spec);
}

void Command::printUsage(const std::string& prefix) const {
	std::string cmdLine = prefix + name;
	if (!aliases.empty()) {
		cmdLine += " (aliases: ";
		for (size_t i = 0; i < aliases.size(); i++) {
			cmdLine += aliases[i];
			if (i < aliases.size() - 1) {
				cmdLine += ", ";
			}
		}
		cmdLine += ")";
	}
	CLI::println((cmdLine + " - " + description).c_str());
	if (!argSpecs.empty()) {
		CLI::println((prefix + "  Arguments:").c_str());
		for (size_t i = 0; i < argSpecs.size(); i++) {
			std::string argLine = prefix + "    -" + argSpecs[i].name + " (";
			switch (argSpecs[i].type) {
			case VAL_INT: argLine += "int"; break;
			case VAL_DOUBLE: argLine += "double"; break;
			case VAL_BOOL: argLine += "bool"; break;
			case VAL_STRING: argLine += "string"; break;
			case VAL_LIST: argLine += "list"; break;
			default: argLine += "unknown"; break;
			}
			argLine += ") ";
			argLine += (argSpecs[i].required ? "required" : "optional");
			if (argSpecs[i].hasDefault) {
				argLine += ", default = " + argSpecs[i].defaultValue.toString();
			}
			if (!argSpecs[i].helpText.empty()) {
				argLine += " -- " + argSpecs[i].helpText;
			}
			CLI::println(argLine.c_str());
		}
	}
	if (!subcommands.empty()) {
		CLI::println((prefix + "  Subcommands:").c_str());
		for (size_t i = 0; i < subcommands.size(); i++) {
			subcommands[i].printUsage(prefix + "    ");
		}
	}
}
