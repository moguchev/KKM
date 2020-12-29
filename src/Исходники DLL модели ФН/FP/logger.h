// Leonid Moguchev (c) 2020
#pragma once

#include "pch.h"

#include <memory>
#include <string>
#include <stdexcept>

template<typename ...Args>
std::string string_format(const std::string& format, Args ...args) {
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size <= 0) {
        throw std::runtime_error("Error during formatting."); 
    }
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

const char ENDL = '\n';

struct ILogger {
    ILogger() = default;
    virtual ~ILogger() = default;
    virtual void log(const std::string& msg) = 0;
    virtual std::string getLog() = 0;
};

class Logger : public ILogger {
public:
    Logger() = default;
    virtual ~Logger() = default;
    virtual void log(const std::string& msg);
    virtual std::string getLog();
    int msg_len();
private:
    std::string _log;
};

