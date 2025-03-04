// src/command.cpp
#include "command.h"
#include "clioutput.h"
#include "dispatcher.h"
#include "RaptorCLI.h"
#include <string>

extern void reportError(CLIOutput* output, const std::string& message);

Command::Command() : name(""), callback(0), output(nullptr), variadic(false) {}

Command::Command(const std::string& cmdName, const std::string& desc)
	: name(cmdName), description(desc), callback(0), output(nullptr), variadic(false) {
}

bool Command::addSubcommand(const Command& cmd) {
	for (size_t i = 0; i < subcommands.size(); i++) {
		if (subcommands[i].name == cmd.name) {
#ifdef USE_DESCRIPTIVE_ERRORS
			reportError(output, "Duplicate subcommand name: " + cmd.name);
#else
			reportError(output, ERROR_CMD_DUPLICATE_NAME);
#endif
			return false;
		}
		for (size_t j = 0; j < cmd.aliases.size(); j++) {
			if (subcommands[i].name == cmd.aliases[j]) {
#ifdef USE_DESCRIPTIVE_ERRORS
				reportError(output, "Duplicate subcommand alias: " + cmd.aliases[j]);
#else
				reportError(output, ERROR_CMD_DUPLICATE_ALIAS);
#endif
				return false;
			}
			for (size_t k = 0; k < subcommands[i].aliases.size(); k++) {
				if (subcommands[i].aliases[k] == cmd.aliases[j]) {
#ifdef USE_DESCRIPTIVE_ERRORS
					reportError(output, "Duplicate subcommand alias: " + cmd.aliases[j]);
#else
					reportError(output, ERROR_CMD_DUPLICATE_ALIAS);
#endif
					return false;
				}
			}
		}
	}
	subcommands.push_back(cmd);
	return true;
}

bool Command::addAlias(const std::string& alias) {
	if (alias == name) {
#ifdef USE_DESCRIPTIVE_ERRORS
		reportError(output, "Alias cannot be the same as the command name: " + alias);
#else
		reportError(output, ERROR_CMD_DUPLICATE_ALIAS);
#endif
		return false;
	}
	for (size_t i = 0; i < aliases.size(); i++) {
		if (aliases[i] == alias) {
#ifdef USE_DESCRIPTIVE_ERRORS
			reportError(output, "Duplicate alias: " + alias);
#else
			reportError(output, ERROR_CMD_DUPLICATE_ALIAS);
#endif
			return false;
		}
	}
	aliases.push_back(alias);
	return true;
}

bool Command::addArgSpec(const ArgSpec& spec) {
	for (size_t i = 0; i < argSpecs.size(); i++) {
		if (argSpecs[i].name == spec.name) {
#ifdef USE_DESCRIPTIVE_ERRORS
			reportError(output, "Duplicate argument name: " + spec.name);
#else
			reportError(output, ERROR_CMD_DUPLICATE_NAME);
#endif
			return false;
		}
	}
	argSpecs.push_back(spec);
	return true;
}

void Command::printUsage(const std::string& prefix, CLIOutput* out) const {
	CLIOutput* outPtr = out ? out : (output ? output : nullptr);
	if (!outPtr) {
		Serial.println((prefix + name + " - " + description).c_str());
	}
	else {
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
		outPtr->println((cmdLine + " - " + description).c_str());
		if (!argSpecs.empty()) {
			outPtr->println((prefix + "  Arguments:").c_str());
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
				outPtr->println(argLine.c_str());
			}
		}
		if (!subcommands.empty()) {
			outPtr->println((prefix + "  Subcommands:").c_str());
			for (size_t i = 0; i < subcommands.size(); i++) {
				subcommands[i].printUsage(prefix + "    ", outPtr);
			}
		}
	}
}

void Command::registerOutput(CLIOutput* out) {
	this->output = out;
	for (auto& subcmd : subcommands) {
		subcmd.registerOutput(out);
	}
}

CLIOutput* Command::getOutput() const {
	return output;
}
