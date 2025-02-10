#ifndef CLI_OUTPUT_H
#define CLI_OUTPUT_H

#ifdef ARDUINO
#include <Arduino.h>
namespace CLI {
	inline void print(const char* s) { Serial.print(s); }
	inline void print(const String& s) { Serial.print(s); }
	inline void println(const char* s) { Serial.println(s); }
	inline void println(const String& s) { Serial.println(s); }
	inline void println() { Serial.println(); }
}
#else
#include <iostream>
#include <string>
namespace CLI {
	inline void print(const std::string& s) { std::cout << s; }
	inline void println(const std::string& s) { std::cout << s << std::endl; }
	inline void println() { std::cout << std::endl; }
}
#endif

#endif