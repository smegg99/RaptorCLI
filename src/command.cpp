// src/command.cpp
#include "command.h"
#include "clioutput.h"

Command::Command() : name(""), callback(0), output(nullptr) {}

Command::Command(const std::string& cmdName, const std::string& desc)
	: name(cmdName), description(desc), callback(0), output(nullptr) {
}

bool Command::addSubcommand(const Command& cmd) {
	for (size_t i = 0; i < subcommands.size(); i++) {
		if (subcommands[i].name == cmd.name) {
			if (output) {
				output->println(("Error: Duplicate subcommand name: " + cmd.name).c_str());
			}
			else {
				Serial.println(("Error: Duplicate subcommand name: " + cmd.name).c_str());
			}
			return false;
		}
		for (size_t j = 0; j < cmd.aliases.size(); j++) {
			if (subcommands[i].name == cmd.aliases[j]) {
				if (output) {
					output->println(("Error: Duplicate subcommand alias: " + cmd.aliases[j]).c_str());
				}
				else {
					Serial.println(("Error: Duplicate subcommand alias: " + cmd.aliases[j]).c_str());
				}
				return false;
			}
			for (size_t k = 0; k < subcommands[i].aliases.size(); k++) {
				if (subcommands[i].aliases[k] == cmd.aliases[j]) {
					if (output) {
						output->println(("Error: Duplicate subcommand alias: " + cmd.aliases[j]).c_str());
					}
					else {
						Serial.println(("Error: Duplicate subcommand alias: " + cmd.aliases[j]).c_str());
					}
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
		if (output) {
			output->println(("Error: Alias cannot be the same as the command name: " + alias).c_str());
		}
		else {
			Serial.println(("Error: Alias cannot be the same as the command name: " + alias).c_str());
		}
		return false;
	}
	for (size_t i = 0; i < aliases.size(); i++) {
		if (aliases[i] == alias) {
			if (output) {
				output->println(("Error: Duplicate alias: " + alias).c_str());
			}
			else {
				Serial.println(("Error: Duplicate alias: " + alias).c_str());
			}
			return false;
		}
	}
	aliases.push_back(alias);
	return true;
}

bool Command::addArgSpec(const ArgSpec& spec) {
	for (size_t i = 0; i < argSpecs.size(); i++) {
		if (argSpecs[i].name == spec.name) {
			if (output) {
				output->println(("Error: Duplicate argument specification: " + spec.name).c_str());
			}
			else {
				Serial.println(("Error: Duplicate argument specification: " + spec.name).c_str());
			}
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
