// include/argument.h
#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <string>
#include <vector>
#include "value.h"

class Argument {
public:
	std::string name;
	std::vector<Value> values;

	Argument() {}
	Argument(const std::string& n) : name(n) {}
};

struct ArgSpec {
	std::string name;
	ValueType type;
	bool required;
	bool hasDefault;
	Value defaultValue;
	std::string helpText;

	ArgSpec(const std::string& n, ValueType t, bool req = false, const std::string& help = "")
		: name(n), type(t), required(req), hasDefault(false), helpText(help) {
	}

	ArgSpec(const std::string& n, ValueType t, bool req, const Value& def, const std::string& help)
		: name(n), type(t), required(req), hasDefault(true), defaultValue(def), helpText(help) {
	}
};

#endif