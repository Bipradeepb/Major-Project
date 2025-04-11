#pragma once

/* --------- Features of this Logger Header Only Library [by pb-dot]------------
Logger Class Following Singleton Design Pattern
1. Checks Environment Variable LOG_ON.[Set to 1 to write to file]
2. Using LOG() macro [Depends on Env Var LOG_ON]:
    - If LOG_ON=1: using the macro ,Logs messages ONLY to file (with timestamp).
    - If LOG_ON is not 1: using the macro ,Logs messages ONLY to terminal (no timestamp).
3. Using LOG_TO() macro :
    - LOG_TO(LogDestination::TERMINAL_ONLY, ...): Logs ONLY to terminal (no timestamp).[Ignores Env Var LOG_ON]
    - LOG_TO(LogDestination::FILE_ONLY, ...): Logs ONLY to file when LOG_ON=1. (with timestamp)
    - LOG_TO(LogDestination::BOTH, ...): Logs to terminal (no timestamp) AND file when LOG_ON=1. (with timestamp).
ie BOTH acts as TERMIBAL_ONLY when LOG_ON not set
4. File log entries are timestamped and include thread ID.

Eg:Very easy to use: include this header file in ur code.
    --> Replacing std::cout << "Hi" << var1 << " age " << var2 << "\n" with;
    --> Prints to Terminal/File [depending on LOG_ON]: LOG("Hi", var1, " age ", var2, "\n");
    --> Explicit terminal: LOG_TO(LogDestination::TERMINAL_ONLY, "Hi", var1, " age ", var2, "\n");
    --> Explicit file:     LOG_TO(LogDestination::FILE_ONLY, "Hi", var1, " age ", var2, "\n");
    --> Explicit both:     LOG_TO(LogDestination::BOTH, "Hi", var1, " age ", var2, "\n");
*/

/* ---- Setting Env Var in Bash
export LOG_ON=1
./my_application
unset LOG_ON # Optional: Unset after running
# OR run temporarily:
LOG_ON=1 ./my_application
*/

// ------ Includes
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <sstream>
#include <chrono>
#include <unistd.h>
#include <iomanip> // For std::put_time
#include <cstdlib> // For getenv
#include <memory>  // For std::unique_ptr (optional, but good practice)
#include <thread>  // For std::this_thread::get_id
#include <stdexcept> // For potential exceptions
#include <utility> // For std::forward

// --- Configuration ---
// Define the name of the environment variable to check
const char* const LOG_ENV_VAR = "LOG_ON";
// Define the name of the log file
const char* const LOG_FILENAME = "Logs/Log.txt";
// --- End Configuration ---

// Enum to specify the logging destination
enum class LogDestination {
    TERMINAL_ONLY,
    FILE_ONLY,
    BOTH
};

class Logger {
public:
    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Get the singleton instance
    static Logger& getInstance() { // static func means called without object
        // C++11 guarantees thread-safe initialization of static local variables
        static Logger instance; // local static var => retains value across func calls
        return instance;
    }

    // --- Overloaded log methods ---

    // 1. Log method WITH explicit destination (ignores environment variable)
    template<typename... Args>
    void log(LogDestination dest, Args&&... args) {
        if(logToFileEnabled_){
            std::lock_guard<std::mutex> lock(logMutex_);
            processLog(dest, std::forward<Args>(args)...);
        } else {
            processLog(dest, std::forward<Args>(args)...);
        }
    }

    // 2. Log method WITHOUT explicit destination (uses environment variable)
    template<typename... Args>
    void log(Args&&... args) {
        // Determine destination based on environment variable setting
        LogDestination defaultDest = logToFileEnabled_ ? LogDestination::FILE_ONLY : LogDestination::TERMINAL_ONLY;
        if(logToFileEnabled_){
            std::lock_guard<std::mutex> lock(logMutex_);
            processLog(defaultDest, std::forward<Args>(args)...);
        } else {
            processLog(defaultDest, std::forward<Args>(args)...);
        }
    }

private:
    std::mutex logMutex_;         // Mutex for thread safety
    std::ofstream logFile_;       // Output file stream
    bool logToFileEnabled_; // Flag indicating if env var set/not and writing to file/not

    // Private processing function (called by public log methods)
    template<typename... Args>
    void processLog(LogDestination effectiveDest, Args&&... args) {
        // Build the core log message using a stringstream
        std::ostringstream message_ss;
        //append all arguments [below 2 lines are equivalent to FOLD expressions(>=c++17)]
        using dummy = int[];
        (void)dummy{0, ((message_ss << std::forward<Args>(args)), 0)...};
        std::string messageContent = message_ss.str();

        // --- Output Logic ---
        bool shouldLogToTerminal = (effectiveDest == LogDestination::TERMINAL_ONLY || effectiveDest == LogDestination::BOTH);
        bool shouldLogToFile = logToFileEnabled_ && (effectiveDest == LogDestination::FILE_ONLY || effectiveDest == LogDestination::BOTH);

        // Print to terminal (NO timestamp)
        if (shouldLogToTerminal) {
            std::cout << messageContent;
            std::cout.flush();
        }

        // Print to file (WITH timestamp, if enabled and requested)
        if (shouldLogToFile) {
            if (logFile_.is_open()) {
                // Get current time FOR FILE LOGGING ONLY
                auto now = std::chrono::system_clock::now();
                auto now_c = std::chrono::system_clock::to_time_t(now);
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

                // Format the timestamp and thread ID
                std::ostringstream timestamp_ss;
                timestamp_ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
                timestamp_ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
                timestamp_ss << " [PID: "<<getpid()<<" Thread:" << std::hash<std::thread::id>{}(std::this_thread::get_id())%1000 << "] ";

                // Combine timestamp and message for the file
                logFile_ << timestamp_ss.str() << messageContent;
                logFile_.flush(); // Ensure data is written immediately
            } else {
                std::cerr << "FATAL LOGGER ERROR: Found log file: " << LOG_FILENAME <<" closed while logging \n";
            }
        }
    }


    // Define the pvt constructor
    Logger() : logToFileEnabled_(false) {
        // Check the environment variable
        const char* env_value = std::getenv(LOG_ENV_VAR);

        if (env_value != nullptr && std::string(env_value) == "1") {
            logToFileEnabled_ = true;
            logFile_.open(LOG_FILENAME, std::ios::out | std::ios::app); // Open in append mode

            if (!logFile_.is_open()) {
                std::cerr << "FATAL LOGGER ERROR: Could not open log file: " << LOG_FILENAME << std::endl;
                logToFileEnabled_ = false; // Disable file logging if open failed
            } else {
                // Log initial messages to file
                logFile_ << "----------------------------------------" << std::endl;
                auto now = std::chrono::system_clock::now();
                auto now_c = std::chrono::system_clock::to_time_t(now);
                logFile_ << "Log session started by PID: "<<getpid()<<" at "
                         << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << std::endl;
                logFile_ << "Logging to file enabled by environment variable: " << LOG_ENV_VAR << "=" << env_value << std::endl;
                logFile_ << "----------------------------------------" << std::endl;
                logFile_.flush();
            }
        } else {
            logToFileEnabled_ = false;
            // Log to console that file logging is disabled
            std::cout << "Logger Info: File logging is disabled (Environment variable '"
                    << LOG_ENV_VAR << "' not set to '1' or not defined)." << std::endl;
        }
    }

    // Define the pvt destructor
    ~Logger() {
        if (logFile_.is_open()) {
             // Log final messages to file
            logFile_ << "----------------------------------------" << std::endl;
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            logFile_ << "Log session ended PID: "<<getpid()<<" at "
                     << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << std::endl;
            logFile_ << "----------------------------------------" << std::endl;
            logFile_.flush();
            logFile_.close();
        }
    }

};

// --- Convenience Macros ---

// Example: LOG("Value is: ", myVar);
#define LOG(...) Logger::getInstance().log(__VA_ARGS__)

// Example: LOG_TO(LogDestination::BOTH, "Value is: ", myVar);
#define LOG_TO(destination, ...) Logger::getInstance().log(destination, __VA_ARGS__)
