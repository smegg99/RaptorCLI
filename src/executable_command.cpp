// src/executable_command.cpp
#include "executable_command.h"
#include "clioutput.h"

ExecutableCommand::ExecutableCommand(const Command& baseCmd, const std::vector<Argument>& presetArgs)
	: baseCommand(baseCmd), presetArgs(presetArgs) {
}

ExecutableCommand::ExecutableCommand(const Command& baseCmd, std::initializer_list<std::pair<std::string, Value>> presetArgsList)
	: baseCommand(baseCmd) {
	for (const auto& p : presetArgsList) {
		Argument arg(p.first);
		arg.values.push_back(p.second);
		presetArgs.push_back(arg);
	}
}

bool ExecutableCommand::execute() const {
	if (baseCommand.callback) {
		Command execCmd = baseCommand;
		execCmd.arguments = presetArgs;
		execCmd.callback(execCmd);
		return true;
	}
	else {
		CLIOutput* out = baseCommand.getOutput();
		if (out) {
			out->println("Error: No callback defined for command: " + baseCommand.name);
		}
		return false;
	}
}

bool ExecutableCommand::executeWithArgs(std::initializer_list<std::pair<std::string, Value>> argsList) const {
	std::vector<Argument> args;
	for (const auto& p : argsList) {
		Argument arg(p.first);
		arg.values.push_back(p.second);
		args.push_back(arg);
	}
	if (baseCommand.callback) {
		Command execCmd = baseCommand;
		execCmd.arguments = args;
		execCmd.callback(execCmd);
		return true;
	}
	else {
		CLIOutput* out = baseCommand.getOutput();
		if (out) {
			out->println("Error: No callback defined for command: " + baseCommand.name);
		}
		return false;
	}
}

// Format the value as a string, including quotes for strings and list brackets for lists.
static std::string formatValue(const Value& val) {
	if (val.type == VAL_STRING) {
		return "\"" + val.stringValue + "\"";
	}
	else if (val.type == VAL_LIST) {
		std::string listStr = "[";
		for (size_t j = 0; j < val.listValue.size(); ++j) {
			if (val.listValue[j].type == VAL_STRING) {
				listStr += "\"" + val.listValue[j].stringValue + "\"";
			}
			else {
				listStr += val.listValue[j].toString();
			}
			if (j < val.listValue.size() - 1)
				listStr += ", ";
		}
		listStr += "]";
		return listStr;
	}
	return val.toString();
}

std::string ExecutableCommand::toString() const {
	std::string result = baseCommand.name;
	for (const auto& arg : presetArgs) {
		result += " -" + arg.name + " ";
		for (size_t i = 0; i < arg.values.size(); i++) {
			result += formatValue(arg.values[i]);
			if (i < arg.values.size() - 1)
				result += ", ";
		}
	}
	return result;
}

std::string ExecutableCommand::toStringWithArgs(std::initializer_list<std::pair<std::string, Value>> argsList) const {
	std::string result = baseCommand.name;
	for (const auto& p : argsList) {
		result += " -" + p.first + " " + formatValue(p.second);
	}
	return result;
}

void ExecutableCommand::toOutputWithArgs(std::initializer_list<std::pair<std::string, Value>> argsList) const {
	std::string cmdStr = toStringWithArgs(argsList);
	CLIOutput* out = baseCommand.getOutput();
	if (out) {
		out->println(cmdStr);
	}
}

const Command& ExecutableCommand::getBaseCommand() const {
	return baseCommand;
}

std::vector<Argument> ExecutableCommand::getPresetArgs() const {
	return presetArgs;
}

void ExecutableCommand::setArgs(const std::vector<Argument>& presetArgs) {
	this->presetArgs = presetArgs;
}

void CommandSequence::addCommand(const ExecutableCommand& cmd) {
	commands.push_back(cmd);
}

bool CommandSequence::execute() const {
	bool overallSuccess = true;
	for (const auto& cmd : commands) {
		bool result = cmd.execute();
		if (!result) {
			overallSuccess = false;
		}
	}
	return overallSuccess;
}

std::string CommandSequence::toString() const {
	std::string result;
	for (size_t i = 0; i < commands.size(); i++) {
		result += commands[i].toString();
		if (i < commands.size() - 1) {
			result += "; ";
		}
	}
	return result;
}
