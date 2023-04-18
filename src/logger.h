#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <filesystem>

class Logger
{
public:
    Logger(std::string log_filename, std::string filename);
    ~Logger();

    template <typename T>
    Logger &operator<<(const T &message)
    {
        std::time_t currentTime = std::time(nullptr);
        std::tm localTime = *std::localtime(&currentTime);

        log_file << "[" << std::put_time(&localTime, "%H:%M:%S-%Y-%m-%d") << "] " << this->filename << ": " << message << std::endl;
        return *this;
    }

    Logger &operator<<(std::ostream &(*manipulator)(std::ostream &))
    {
        log_file << manipulator;
        return *this;
    }

private:
    std::ofstream log_file;
    std::string filename;
};
