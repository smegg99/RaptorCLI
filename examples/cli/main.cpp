#include <iostream>
#include <vector>
#include "RaptorCLI.h"

// Callback for the "calc" command.
// Expects three arguments:
//  -a : double (first operand)
//  -b : double (second operand)
//  -op: string (operator: +, -, *, /)
void calcCallback(const Command & cmd) {
	double a = 0.0, b = 0.0;
	std::string op;
	for (size_t i = 0; i < cmd.arguments.size(); i++) {
		if (cmd.arguments[i].name == "a")
			a = cmd.arguments[i].values[0].doubleValue;
		else if (cmd.arguments[i].name == "b")
			b = cmd.arguments[i].values[0].doubleValue;
		else if (cmd.arguments[i].name == "op")
			op = cmd.arguments[i].values[0].stringValue;
	}
	std::cout << "[calc] " << a << " " << op << " " << b << " = ";
	if (op == "+")
		std::cout << (a + b);
	else if (op == "-")
		std::cout << (a - b);
	else if (op == "*")
		std::cout << (a * b);
	else if (op == "/") {
		if (b == 0.0)
			std::cout << "Division by zero error";
		else
			std::cout << (a / b);
	}
	else
		std::cout << "Unknown operator: " << op;
	std::cout << std::endl;
}

// Callback for the "do" command.
// Expects one argument:
//  -v : int
void doCallback(const Command & cmd) {
	std::cout << "[do] command executed!" << std::endl;
	for (size_t i = 0; i < cmd.arguments.size(); i++) {
		std::cout << "  " << cmd.arguments[i].name << " = "
			<< cmd.arguments[i].values[0].toString() << std::endl;
	}
}

// Callback for the "action" subcommand under "do".
// Expects one argument:
//  -x : string
void actionCallback(const Command & cmd) {
	std::cout << "[do action] subcommand executed!" << std::endl;
	for (size_t i = 0; i < cmd.arguments.size(); i++) {
		std::cout << "  " << cmd.arguments[i].name << " = "
			<< cmd.arguments[i].values[0].toString() << std::endl;
	}
}

// Callback for the "printlist" command.
// Expects one argument:
//  -items : list
void printListCallback(const Command & cmd) {
	std::cout << "[printlist] command executed!" << std::endl;
	for (size_t i = 0; i < cmd.arguments.size(); i++) {
		if (cmd.arguments[i].name == "items") {
			Value listVal = cmd.arguments[i].values[0];
			if (listVal.type != VAL_LIST) {
				std::cout << "Error: 'items' is not a list." << std::endl;
				return;
			}
			std::cout << "List elements:" << std::endl;
			for (size_t j = 0; j < listVal.listValue.size(); j++) {
				std::cout << "  Element " << j << ": "
					<< listVal.listValue[j].toString() << std::endl;
			}
			return;
		}
	}
	std::cout << "No 'items' argument provided." << std::endl;
}

int main() {
	Dispatcher dispatcher;

	// Register "calc" command.
	// Expects: -a (double), -b (double), -op (string)
	Command calcCmd("calc", "Performs arithmetic operations");
	calcCmd.callback = calcCallback;
	calcCmd.addArgSpec(ArgSpec("a", VAL_DOUBLE, true, "First operand (double)"));
	calcCmd.addArgSpec(ArgSpec("b", VAL_DOUBLE, true, "Second operand (double)"));
	calcCmd.addArgSpec(ArgSpec("op", VAL_STRING, true, "Operator (+, -, *, /)"));
	dispatcher.registerCommand(calcCmd);

	// Register "do" command.
	// Expects: -v (int)
	Command doCmd("do", "Performs a simple action");
	doCmd.callback = doCallback;
	doCmd.addArgSpec(ArgSpec("v", VAL_INT, true, "An integer value"));

	// Register subcommand "action" under "do".
	// Expects: -x (string)
	Command actionCmd("action", "Performs a detailed action");
	actionCmd.callback = actionCallback;
	actionCmd.addArgSpec(ArgSpec("x", VAL_STRING, true, "Action description"));
	actionCmd.addAlias("act");
	doCmd.addSubcommand(actionCmd);
	dispatcher.registerCommand(doCmd);

	// Register "printlist" command.
	// Expects: -items (list)
	Command printListCmd("printlist", "Prints a list of items");
	printListCmd.callback = printListCallback;
	printListCmd.addArgSpec(ArgSpec("items", VAL_LIST, true, "List of items to print"));
	printListCmd.addAlias("pl");
	dispatcher.registerCommand(printListCmd);

	std::string input;
	while (true) {
		std::cout << "> ";
		std::getline(std::cin, input);
		if (input == "exit") {
			break;
		}
		try {
			dispatcher.dispatch(input);
		}
		catch (std::exception& ex) {
			std::cout << "Error: " << ex.what() << std::endl;
		}
	}
	std::cout << "CLI terminated." << std::endl;
	return 0;
}
