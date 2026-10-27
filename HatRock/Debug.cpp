#include "Debug.h"
std::string Debug::fileName = ""; // Initialize the static member variable

void Debug::DebugInit(const std::string& fileName_) {
	fileName = fileName + ".txt"; // Append ".txt" to the provided file name
	std::ofstream out;
	out.open(fileName, std::ios::out);
	out << ""; // Clear the file content
	out.close();
} // this method initializes the debug system by setting the log file name and clearing its content. 
// It appends ".txt" to the provided file name, opens the file in output mode, writes an empty string 
// to clear any existing content, and then closes the file.

void Debug::Log(const MessageType type_, const std::string& message_, const std::string& fileName_, const int line_) {
	std::ofstream out; // Create an output file stream
	std::string msg; // Create a string to hold the log message
	out.open(fileName, std::ios::out | std::ios::app); // Open the log file in append mode. ios means input/output, app means append
	if (type_ == TYPE_INFO) {
		msg = message_;
	}
	else {
		msg = message_ + " in file: " + fileName_ + " at line: " + std::to_string(line_);
	}
#ifdef _DEBUG
	std::cout << msg << std::endl; // Print the message to the console in debug mode
#endif
	out << msg << std::endl; // Write the message to the log file
	out.flush(); // Flush the output buffer to ensure the message is written to the file
	out.close();
}

void Debug::Info(const std::string& message_, const std::string& fileName_, const int line_) {
	Log(MessageType::TYPE_INFO, "[INFO]: " + message_, fileName_, line_); 
} 

void Debug::Trace(const std::string& message_, const std::string& fileName_, const int line_) {
	Log(MessageType::TYPE_TRACE, "[TRACE]: " + message_, fileName_, line_);
}

void Debug::Warning(const std::string& message_, const std::string& fileName_, const int line_) {
	Log(MessageType::TYPE_WARNING, "[WARNING]: " + message_, fileName_, line_);
}

void Debug::Error(const std::string& message_, const std::string& fileName_, const int line_) {
	Log(MessageType::TYPE_ERROR, "[ERROR]: " + message_, fileName_, line_);
}

void Debug::FatalError(const std::string& message_, const std::string& fileName_, const int line_) {
	Log(MessageType::TYPE_FATAL_ERROR, "[FATAL ERROR]: " + message_, fileName_, line_);
}