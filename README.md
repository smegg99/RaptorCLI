# RaptorCLI Library

RaptorCLI is a lightweight and simple, C++98 compatible command-line parser that works on the Arduino framework. It lets you easily define commands (with subcommands, arguments, and aliases) and automatically provides a built-in help command. The library also abstracts output so that Serial is used on Arduino and std::cout otherwise. It was designed to be used on the ESP32-S3-MINI-1 module and is a part of the [LoRaptor](https://github.com/smegg99/LoRaptor) project.

## Features

- **Command Hierarchy:** Define commands and subcommands with aliases.
- **Argument Parsing:** Supports int, double, bool, string, and list arguments.
- **Built-in Help:** Global help command (`help` or `?`) displays usage info.
- **Cross-Platform Output:** Uses Serial on Arduino, std::cout on other platforms.
- **Error Handling:** Throws descriptive exceptions for missing, duplicate, or type-mismatched arguments.

## How It Works

- **Value:** Represents a parsed value (int, double, bool, string, or list).
- **Argument & ArgSpec:**  
  - **Argument:** Holds the parsed value(s) for a command argument.  
  - **ArgSpec:** Declares an expected argument (its type, requirement, optional default, and help text).
- **Command:** Represents a command with a name, description, aliases, subcommands, expected arguments, and a callback function.
- **Dispatcher:** Manages registered commands, parses input, validates arguments, and dispatches the appropriate callbacks.
- **CLIOutput:** An output abstraction layer (Serial on Arduino, std::cout on PC).

## Example

You can find a basic command-line interface (CLI) example [here](examples/cli).
Compile it from the library root with:

```bash
g++ -std=c++98 -I./include -o cli_example examples/cli/main.cpp src/Command.cpp src/Dispatcher.cpp 
```
