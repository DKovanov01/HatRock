#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>	
#include <string>
#include <fstream>
#include <ctime> // for std::time_t, std::tm, std::localtime, std::strftime

enum MessageType : unsigned short {
	TYPE_NONE = 0,
	TYPE_FATAL_ERROR,
	TYPE_ERROR,
	TYPE_WARNING,
	TYPE_TRACE,
	TYPE_INFO
};

class Debug {
public:
	Debug(const Debug&) = delete; // Delete copy constructor
	Debug(Debug&&) = delete; // Delete move constructor
	Debug& operator=(const Debug&) = delete; // Delete copy assignment operator
	Debug& operator=(Debug&&) = delete; // Delete move assignment operator

	Debug() = delete; // Delete default constructor

	static void DebugInit(const std::string& fileName_); // Initialize the debug system with a log file name
	static void Info(const std::string& message_,
		const std::string& fileName_, const int line_); // Log an info message
	static void Trace(const std::string& message_,
		const std::string& fileName_, const int line_); // Log a trace message
	static void Warning(const std::string& message_,
		const std::string& fileName_, const int line_); // Log a warning message
	static void Error(const std::string& message_,
		const std::string& fileName_, const int line_); // Log an error message
	static void FatalError(const std::string& message_,
		const std::string& fileName_, const int line_); // Log a fatal error message

private:
	static void Log(const MessageType type_, 
		const std::string& message_,
		const std::string& fileName_, 
		const int line_); // Log a message with a specific type
	static std::string fileName; // Name of the log file
};

#endif // DEBUG_H