#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <vector>
#include <cstdio>

enum ValueType {
	VAL_NONE,
	VAL_INT,
	VAL_DOUBLE,
	VAL_BOOL,
	VAL_STRING,
	VAL_LIST
};

class Value {
public:
	ValueType type;
	int         intValue;
	double      doubleValue;
	bool        boolValue;
	std::string stringValue;
	std::vector<Value> listValue;

	Value()
		: type(VAL_NONE), intValue(0), doubleValue(0.0), boolValue(false) {
	}

	Value(int v)
		: type(VAL_INT), intValue(v), doubleValue(0.0), boolValue(false) {
	}

	Value(double v)
		: type(VAL_DOUBLE), intValue(0), doubleValue(v), boolValue(false) {
	}

	Value(bool v)
		: type(VAL_BOOL), intValue(0), doubleValue(0.0), boolValue(v) {
	}

	Value(const std::string& v)
		: type(VAL_STRING), intValue(0), doubleValue(0.0), boolValue(false), stringValue(v) {
	}

	Value(const char* v)
		: type(VAL_STRING), intValue(0), doubleValue(0.0), boolValue(false), stringValue(v) {
	}

	Value(const std::vector<Value>& v)
		: type(VAL_LIST), intValue(0), doubleValue(0.0), boolValue(false), listValue(v) {
	}

	std::string toString() const {
		char buffer[32];
		switch (type) {
		case VAL_INT:
			std::sprintf(buffer, "%d", intValue);
			return std::string(buffer);
		case VAL_DOUBLE:
			std::sprintf(buffer, "%f", doubleValue);
			return std::string(buffer);
		case VAL_BOOL:
			return boolValue ? "true" : "false";
		case VAL_STRING:
			return stringValue;
		case VAL_LIST: {
			std::string result = "[";
			for (size_t i = 0; i < listValue.size(); i++) {
				result += listValue[i].toString();
				if (i < listValue.size() - 1)
					result += ", ";
			}
			result += "]";
			return result;
		}
		default:
			return "";
		}
	}
};

#endif
