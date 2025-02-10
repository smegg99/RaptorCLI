#include "Dispatcher.h"
#include "CLIOutput.h"
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <set>

#define QUOTE_CHAR '"'  
#define ESCAPE_CHAR '\\'  
#define DASH_CHAR '-'
#define DECIMAL_POINT '.'
#define DELIMITER_CHAR ','
#define NULL_CHAR '\0'
#define LIST_START '['  
#define LIST_END ']'  
#define HELP_FLAG_SHORT "h"  
#define HELP_FLAG_LONG "help"  

enum TokenizerState { TS_OUTSIDE, TS_IN_QUOTE, TS_IN_ESCAPE, TS_IN_LIST };

static Dispatcher* gDispatcher = 0;

static void helpCommandCallback(const Command& cmd) {
	if (gDispatcher) {
		gDispatcher->printGlobalHelp();
	}
}

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
	Command helpCmd("help", "Displays help information for all commands.");
	helpCmd.addAlias("?");
	helpCmd.callback = helpCommandCallback;
	registerCommand(helpCmd);
}

void Dispatcher::registerCommand(const Command& cmd) {
	for (size_t i = 0; i < commands.size(); i++) {
		if (commands[i].name == cmd.name) {
			throw std::runtime_error("Duplicate command name: " + cmd.name);
		}
		for (size_t j = 0; j < cmd.aliases.size(); j++) {
			if (commands[i].name == cmd.aliases[j]) {
				throw std::runtime_error("Duplicate command alias: " + cmd.aliases[j]);
			}
			for (size_t k = 0; k < commands[i].aliases.size(); k++) {
				if (commands[i].aliases[k] == cmd.aliases[j]) {
					throw std::runtime_error("Duplicate command alias: " + cmd.aliases[j]);
				}
			}
		}
	}
	commands.push_back(cmd);
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
					tokens.push_back(token);
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
		tokens.push_back(token);
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

void Dispatcher::parseArguments(const std::vector<std::string>& tokens, size_t index, std::vector<Argument>& outArgs) {
	std::set<std::string> seenArgs;
	while (index < tokens.size()) {
		std::string token = tokens[index];
		if (token.empty() || !isFlagToken(token)) {
			throw std::runtime_error("Unexpected token: " + token);
		}
		std::string argName = token.substr(1);
		if (seenArgs.find(argName) != seenArgs.end()) {
			throw std::runtime_error("Duplicate argument: " + argName);
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
	std::vector<std::string> tokens = tokenize(input);
	size_t index = 0;
	const Command* cmd = matchCommand(tokens, index);
	if (!cmd) {
		throw std::runtime_error("Unknown command.");
	}
	if (index < tokens.size() && (tokens[index].empty() || !isFlagToken(tokens[index]))) {
		throw std::runtime_error("Unexpected token: " + tokens[index]);
	}
	std::vector<Argument> parsedArgs;
	parseArguments(tokens, index, parsedArgs);

	bool foundHelpShort = false, foundHelpLong = false;
	for (size_t i = 0; i < parsedArgs.size(); i++) {
		if (parsedArgs[i].name == HELP_FLAG_SHORT)
			foundHelpShort = true;
		if (parsedArgs[i].name == HELP_FLAG_LONG)
			foundHelpLong = true;
	}
	if (foundHelpShort && foundHelpLong) {
		throw std::runtime_error("Duplicate help flag: both -h and -help provided.");
	}
	if (foundHelpShort || foundHelpLong) {
		cmd->printUsage();
		return true;
	}

	std::vector<Argument> mergedArgs;
	for (size_t i = 0; i < cmd->argSpecs.size(); i++) {
		const ArgSpec& spec = cmd->argSpecs[i];
		bool found = false;
		for (size_t j = 0; j < parsedArgs.size(); j++) {
			if (parsedArgs[j].name == spec.name) {
				if (parsedArgs[j].values.empty()) {
					throw std::runtime_error("Argument " + spec.name + " has no value.");
				}
				Value provided = parsedArgs[j].values[0];
				if (spec.type == VAL_DOUBLE) {
					if (provided.type == VAL_INT) {
						provided.doubleValue = (double)provided.intValue;
						provided.type = VAL_DOUBLE;
					}
					else if (provided.type != VAL_DOUBLE) {
						throw std::runtime_error("Type mismatch for argument: " + spec.name);
					}
				}
				else if (provided.type != spec.type) {
					throw std::runtime_error("Type mismatch for argument: " + spec.name);
				}
				parsedArgs[j].values[0] = provided;
				mergedArgs.push_back(parsedArgs[j]);
				found = true;
				break;
			}
		}
		if (!found) {
			if (spec.required && !spec.hasDefault) {
				throw std::runtime_error("Required argument missing: " + spec.name);
			}
			if (spec.hasDefault) {
				Argument defaultArg(spec.name);
				defaultArg.values.push_back(spec.defaultValue);
				mergedArgs.push_back(defaultArg);
			}
		}
	}
	for (size_t j = 0; j < parsedArgs.size(); j++) {
		bool recognized = false;
		for (size_t i = 0; i < cmd->argSpecs.size(); i++) {
			if (parsedArgs[j].name == cmd->argSpecs[i].name) {
				recognized = true;
				break;
			}
		}
		if (!recognized) {
			throw std::runtime_error("Unknown argument provided: " + parsedArgs[j].name);
		}
	}

	Command execCmd = *cmd;
	execCmd.arguments = mergedArgs;
	if (execCmd.callback) {
		execCmd.callback(execCmd);
		return true;
	}
	return false;
}

void Dispatcher::printGlobalHelp() const {
	CLI::println("Global Help:");
	for (size_t i = 0; i < commands.size(); i++) {
		commands[i].printUsage("  ");
	}
}
