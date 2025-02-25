// include/clioutput.h
#ifndef CLI_OUTPUT_H
#define CLI_OUTPUT_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <iostream>
#include <string>
#endif

class CLIOutput {
public:
	virtual void print(const std::string& s) = 0;
	virtual void println(const std::string& s) = 0;
	virtual void println() = 0;
	virtual ~CLIOutput() = default;
};

#ifdef ARDUINO
class ArduinoCLIOutput : public CLIOutput {
public:
	void print(const std::string& s) override { Serial.print(s.c_str()); }
	void println(const std::string& s) override { Serial.println(s.c_str()); }
	void println() override { Serial.println(); }
};
#else
class StdCLIOutput : public CLIOutput {
public:
	void print(const std::string& s) override { std::cout << s; }
	void println(const std::string& s) override { std::cout << s << std::endl; }
	void println() override { std::cout << std::endl; }
};
#endif

#endif