// Leonid Moguchev (c) 2020
#include "pch.h"
#include "logger.h"

void Logger::log(const std::string& msg) {
    _log += msg + ENDL;
}

std::string Logger::getLog() {
    auto tmp = _log;
    _log.clear();
    return tmp;
}

int Logger::msg_len() {
    return _log.length();
}