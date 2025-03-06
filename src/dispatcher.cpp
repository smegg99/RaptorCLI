// src/dispatcher.cpp
#include "dispatcher.h"
#include "clioutput.h"
#include "RaptorCLI.h"
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <set>

namespace {
	static Dispatcher* gDispatcher = nullptr;
}

#define QUOTE_CHAR '"'  
#define ESCAPE_CHAR '\\'  
#define DASH_CHAR '-'
#define DECIMAL_POINT '.'
#define DELIMITER_CHAR ','
#define NULL_CHAR '\0'
#define HELP_FLAG_SHORT "h"  
#define HELP_FLAG_LONG "help"
#define LIST_START '['
#define LIST_END ']'
#define COMMAND_DELIMITER ';'

// Helper function to trim whitespace from both ends of a string.
static std::string trim(const std::string& s) {
	size_t start = 0;
	while (start < s.size() && std::isspace(s[start]))
		start++;
	size_t end = s.size();
	while (end > start && std::isspace(s[end - 1]))
		end--;
	return s.substr(start, end - start);
}

// Splits input string into separate commands using ';' as delimiter.
enum SplitState { SS_OUTSIDE, SS_IN_QUOTE, SS_IN_ESCAPE, SS_IN_LIST };
static std::vector<std::string> splitCommands(const std::string& input) {
	std::vector<std::string> commands;
	std::string current;
	SplitState state = SS_OUTSIDE;
	for (char c : input) {
		switch (state) {
		case SS_OUTSIDE:
			if (c == COMMAND_DELIMITER) {
				commands.push_back(current);
				current.clear();
			}
			else if (c == QUOTE_CHAR) {
				state = SS_IN_QUOTE;
				current.push_back(c);
			}
			else if (c == LIST_START) {
				state = SS_IN_LIST;
				current.push_back(c);
			}
			else {
				current.push_back(c);
			}
			break;
		case SS_IN_QUOTE:
			if (c == ESCAPE_CHAR) {
				state = SS_IN_ESCAPE;
				current.push_back(c);
			}
			else if (c == QUOTE_CHAR) {
				state = SS_OUTSIDE;
				current.push_back(c);
			}
			else {
				current.push_back(c);
			}
			break;
		case SS_IN_ESCAPE:
			current.push_back(c);
			state = SS_IN_QUOTE;
			break;
		case SS_IN_LIST:
			current.push_back(c);
			if (c == LIST_END) {
				state = SS_OUTSIDE;
			}
			break;
		}
	}
	if (!current.empty()) {
		commands.push_back(current);
	}
	return commands;
}

// Helper to report an error via the registered output (or Serial as fallback, just in case).
void Dispatcher::reportError(const std::string& msg) {
#ifdef USE_DESCRIPTIVE_ERRORS
	if (output) {
		output->println(msg);
	}
#else
	if (errorCallback) {
		errorCallback(msg);
	}
#endif
}

void Dispatcher::registerErrorCallback(ErrorCallback callback) {
	errorCallback = callback;
}

bool Dispatcher::dispatchSingleCommand(const std::string& command) {
	std::vector<std::string> tokens = tokenize(command);
	size_t index = 0;
	const Command* cmd = matchCommand(tokens, index);
	if (!cmd) {
#ifdef USE_DESCRIPTIVE_ERRORS
		reportError("Unknown command: " + tokens[0]);
#else
		reportError(ERROR_CMD_UNKNOWN);
#endif
		return false;
	}
	if (index < tokens.size() && (tokens[index].empty() || tokens[index][0] != DASH_CHAR)) {
#ifdef USE_DESCRIPTIVE_ERRORS
		reportError("Unexpected token: " + tokens[index]);
#else
		reportError(ERROR_CMD_UNEXPECTED_TOKEN);
#endif
		return false;
	}
	std::vector<Argument> parsedArgs;
	if (!parseArguments(tokens, index, parsedArgs)) {
		return false;
	}
	bool foundHelpShort = false, foundHelpLong = false;
	for (size_t i = 0; i < parsedArgs.size(); i++) {
		if (parsedArgs[i].name == HELP_FLAG_SHORT)
			foundHelpShort = true;
		if (parsedArgs[i].name == HELP_FLAG_LONG)
			foundHelpLong = true;
	}
	if (foundHelpShort && foundHelpLong) {
#ifdef USE_DESCRIPTIVE_ERRORS
		reportError("Duplicate help flag: both -h and -help provided.");
#else
		reportError(ERROR_CMD_DUPLICATE_HELP_FLAG);
#endif
		return false;
	}
	if (foundHelpShort || foundHelpLong) {
		cmd->printUsage("", output);
		return true;
	}
	std::vector<Argument> mergedArgs;
	for (size_t i = 0; i < cmd->argSpecs.size(); i++) {
		const ArgSpec& spec = cmd->argSpecs[i];
		bool found = false;
		for (size_t j = 0; j < parsedArgs.size(); j++) {
			if (parsedArgs[j].name == spec.name) {
				if (parsedArgs[j].values.empty()) {
#ifdef USE_DESCRIPTIVE_ERRORS
					reportError("Argument " + spec.name + " has no value.");
#else
					reportError(ERROR_CMD_MISSING_REQUIRED_ARG);
#endif
					return false;
				}
				Value provided = parsedArgs[j].values[0];
				if (spec.type == VAL_DOUBLE) {
					if (provided.type == VAL_INT) {
						provided.doubleValue = (double)provided.intValue;
						provided.type = VAL_DOUBLE;
					}
					else if (provided.type != VAL_DOUBLE) {
#ifdef USE_DESCRIPTIVE_ERRORS
						reportError("Type mismatch for argument: " + spec.name);
#else
						reportError(ERROR_CMD_TYPE_MISMATCH);
#endif
						return false;
					}
				}
				else if (provided.type != spec.type) {
#ifdef USE_DESCRIPTIVE_ERRORS
					reportError("Type mismatch for argument: " + spec.name);
#else
					reportError(ERROR_CMD_TYPE_MISMATCH);
#endif
					return false;
				}
				parsedArgs[j].values[0] = provided;
				mergedArgs.push_back(parsedArgs[j]);
				found = true;
				break;
			}
		}
		if (!found) {
			if (spec.required && !spec.hasDefault) {
#ifdef USE_DESCRIPTIVE_ERRORS
				reportError("Required argument missing: " + spec.name);
#else
				reportError(ERROR_CMD_MISSING_REQUIRED_ARG);
#endif
				return false;
			}
			if (spec.hasDefault) {
				Argument defaultArg(spec.name);
				defaultArg.values.push_back(spec.defaultValue);
				mergedArgs.push_back(defaultArg);
			}
		}
	}
	Command execCmd = *cmd;
	execCmd.arguments = mergedArgs;
	if (execCmd.callback) {
		execCmd.callback(execCmd);
		return true;
	}
	else {
#ifdef USE_DESCRIPTIVE_ERRORS
		reportError("No callback defined for command: " + execCmd.name);
#else
		reportError(ERROR_CMD_NO_CALLBACK);
#endif
		return false;
	}
}

enum TokenizerState { TS_OUTSIDE, TS_IN_QUOTE, TS_IN_ESCAPE, TS_IN_LIST };

static bool isFlagToken(const std::string& token) {
	if (token.empty())
		return false;
	if (token[0] != DASH_CHAR)
		return false;
	if (token.size() == 1)
		return true;
	if (std::isdigit(token[1]) || token[1] == DECIMAL_POINT)
		return false;
	return true;
}

static std::vector<std::string> splitListItems(const std::string& input) {
	std::vector<std::string> items;
	std::string current;
	enum { ST_OUT, ST_IN_QUOTE, ST_IN_ESCAPE } state = ST_OUT;
	for (size_t i = 0; i < input.size(); i++) {
		char c = input[i];
		switch (state) {
		case ST_OUT:
			if (c == ESCAPE_CHAR) {
				state = ST_IN_ESCAPE;
			}
			else if (c == QUOTE_CHAR) {
				state = ST_IN_QUOTE;
				current.push_back(c);
			}
			else if (c == DELIMITER_CHAR) {
				items.push_back(current);
				current.clear();
			}
			else {
				current.push_back(c);
			}
			break;
		case ST_IN_QUOTE:
			if (c == ESCAPE_CHAR) {
				state = ST_IN_ESCAPE;
				current.push_back(c);
			}
			else if (c == QUOTE_CHAR) {
				state = ST_OUT;
				current.push_back(c);
			}
			else {
				current.push_back(c);
			}
			break;
		case ST_IN_ESCAPE:
			current.push_back(c);
			state = ST_IN_QUOTE;
			break;
		}
	}
	if (!current.empty()) {
		items.push_back(current);
	}
	return items;
}

Dispatcher::Dispatcher() {
	gDispatcher = this;
}

void Dispatcher::registerOutput(CLIOutput* output) {
	this->output = output;
}

bool Dispatcher::registerCommand(const Command& cmd) {
	for (size_t i = 0; i < commands.size(); i++) {
		if (commands[i].name == cmd.name) {
#ifdef USE_DESCRIPTIVE_ERRORS
			reportError("Duplicate command name: " + cmd.name);
#else
			reportError(ERROR_CMD_DUPLICATE_NAME);
#endif
			return false;
		}
		for (size_t j = 0; j < cmd.aliases.size(); j++) {
			if (commands[i].name == cmd.aliases[j]) {
#ifdef USE_DESCRIPTIVE_ERRORS
				reportError("Duplicate command alias: " + cmd.aliases[j]);
#else
				reportError(ERROR_CMD_DUPLICATE_ALIAS);
#endif
				return false;
			}
			for (size_t k = 0; k < commands[i].aliases.size(); k++) {
				if (commands[i].aliases[k] == cmd.aliases[j]) {
#ifdef USE_DESCRIPTIVE_ERRORS
					reportError("Duplicate command alias: " + cmd.aliases[j]);
#else
					reportError(ERROR_CMD_DUPLICATE_ALIAS);
#endif
					return false;
				}
			}
		}
	}
	commands.push_back(cmd);
	return true;
}

std::vector<std::string> Dispatcher::tokenize(const std::string& input) {
	std::vector<std::string> tokens;
	std::string token;
	TokenizerState state = TS_OUTSIDE;
	for (size_t i = 0; i < input.size(); i++) {
		char c = input[i];
		switch (state) {
		case TS_OUTSIDE:
			if (isspace(c)) {
				if (!token.empty()) {
					tokens.push_back(trim(token));
					token.clear();
				}
			}
			else if (c == QUOTE_CHAR) {
				state = TS_IN_QUOTE;
			}
			else if (c == LIST_START) {
				state = TS_IN_LIST;
				token.push_back(c);
			}
			else {
				token.push_back(c);
			}
			break;
		case TS_IN_QUOTE:
			if (c == ESCAPE_CHAR) {
				state = TS_IN_ESCAPE;
			}
			else if (c == QUOTE_CHAR) {
				state = TS_OUTSIDE;
				break;
			}
			else {
				token.push_back(c);
			}
			break;
		case TS_IN_ESCAPE:
			token.push_back(c);
			state = TS_IN_QUOTE;
			break;
		case TS_IN_LIST:
			token.push_back(c);
			if (c == LIST_END) {
				state = TS_OUTSIDE;
			}
			break;
		}
	}
	if (!token.empty()) {
		tokens.push_back(trim(token));
	}
	return tokens;
}

const Command* Dispatcher::matchCommand(const std::vector<std::string>& tokens, size_t& index) {
	if (tokens.empty())
		return 0;

	for (size_t i = 0; i < commands.size(); i++) {
		if (commands[i].name == tokens[0]) {
			const Command* current = &commands[i];
			index = 1;
			while (index < tokens.size() && tokens[index][0] != DASH_CHAR) {
				bool foundSub = false;
				for (size_t j = 0; j < current->subcommands.size(); j++) {
					if (current->subcommands[j].name == tokens[index]) {
						current = &current->subcommands[j];
						foundSub = true;
						index++;
						break;
					}
					for (size_t k = 0; k < current->subcommands[j].aliases.size(); k++) {
						if (current->subcommands[j].aliases[k] == tokens[index]) {
							current = &current->subcommands[j];
							foundSub = true;
							index++;
							break;
						}
					}
					if (foundSub)
						break;
				}
				if (!foundSub)
					break;
			}
			return current;
		}
		for (size_t j = 0; j < commands[i].aliases.size(); j++) {
			if (commands[i].aliases[j] == tokens[0]) {
				const Command* current = &commands[i];
				index = 1;
				while (index < tokens.size() && tokens[index][0] != DASH_CHAR) {
					bool foundSub = false;
					for (size_t k = 0; k < current->subcommands.size(); k++) {
						if (current->subcommands[k].name == tokens[index]) {
							current = &current->subcommands[k];
							foundSub = true;
							index++;
							break;
						}
						for (size_t l = 0; l < current->subcommands[k].aliases.size(); l++) {
							if (current->subcommands[k].aliases[l] == tokens[index]) {
								current = &current->subcommands[k];
								foundSub = true;
								index++;
								break;
							}
						}
						if (foundSub)
							break;
					}
					if (!foundSub)
						break;
				}
				return current;
			}
		}
	}
	return 0;
}

bool Dispatcher::parseArguments(const std::vector<std::string>& tokens, size_t index, std::vector<Argument>& outArgs) {
	std::set<std::string> seenArgs;
	while (index < tokens.size()) {
		std::string token = tokens[index];
		if (token.empty() || !isFlagToken(token)) {
#ifdef USE_DESCRIPTIVE_ERRORS
			reportError("Unexpected token: " + token);
#else
			reportError(ERROR_CMD_UNEXPECTED_TOKEN);
#endif
			return false;
		}
		std::string argName = token.substr(1);
		if (seenArgs.find(argName) != seenArgs.end()) {
#ifdef USE_DESCRIPTIVE_ERRORS
			reportError("Duplicate argument: " + argName);
#else
			reportError(ERROR_CMD_DUPLICATE_NAME);
#endif
			return false;
		}
		seenArgs.insert(argName);
		Argument arg(argName);
		index++;
		while (index < tokens.size() && !isFlagToken(tokens[index])) {
			std::string valueToken = tokens[index];
			if (!valueToken.empty() && valueToken[0] == LIST_START) {
				arg.values.push_back(parseList(valueToken));
			}
			else {
				arg.values.push_back(parseValue(valueToken));
			}
			index++;
		}
		outArgs.push_back(arg);
	}
	return true;
}

Value Dispatcher::parseValue(const std::string& token) {
	char* endptr = 0;
	long intValue = std::strtol(token.c_str(), &endptr, 10);
	if (endptr != token.c_str() && *endptr == NULL_CHAR) {
		return Value((int)intValue);
	}
	double doubleValue = std::strtod(token.c_str(), &endptr);
	if (endptr != token.c_str() && *endptr == NULL_CHAR) {
		return Value(doubleValue);
	}
	if (token == "true")
		return Value(true);
	else if (token == "false")
		return Value(false);
	return Value(token);
}

Value Dispatcher::parseList(const std::string& token) {
	std::vector<Value> listValues;
	if (token.size() >= 2 && token[0] == LIST_START && token[token.size() - 1] == LIST_END) {
		std::string inner = token.substr(1, token.size() - 2);
		std::vector<std::string> items = splitListItems(inner);
		for (size_t i = 0; i < items.size(); i++) {
			std::string item = items[i];
			size_t start = 0;
			while (start < item.size() && isspace(item[start]))
				start++;
			size_t end = item.size();
			while (end > start && isspace(item[end - 1]))
				end--;
			std::string trimmed = item.substr(start, end - start);
			if (!trimmed.empty()) {
				if (trimmed[0] == QUOTE_CHAR && trimmed[trimmed.size() - 1] == QUOTE_CHAR) {
					trimmed = trimmed.substr(1, trimmed.size() - 2);
				}
				listValues.push_back(parseValue(trimmed));
			}
		}
	}
	return Value(listValues);
}

bool Dispatcher::dispatch(const std::string& input) {
	std::string cleanedInput = trim(input);
	std::vector<std::string> commandStrings = splitCommands(cleanedInput);
	bool overallSuccess = true;
	for (const auto& cmdStr : commandStrings) {
		std::string trimmedCmd = trim(cmdStr);
		if (trimmedCmd.empty())
			continue;
		bool result = dispatchSingleCommand(trimmedCmd);
		if (!result)
			overallSuccess = false;
	}
	return overallSuccess;
}

void Dispatcher::printGlobalHelp() const {
	for (size_t i = 0; i < commands.size(); i++) {
		commands[i].printUsage("  ", output);
	}
}

CLIOutput* Dispatcher::getOutput() {
	return output;
}
