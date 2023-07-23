#include "logger.h"

#include <utility>

Logger::Logger(std::string log_filename, std::string filename)
    : filename(std::move(filename))
{
    std::string dir_path = "logs";
    if(!std::filesystem::exists(dir_path)) {
        if(!std::filesystem::create_directory(dir_path))
            std::cerr << "Failed to create logs directory \"" << dir_path
                      << "\"" << std::endl;
    }

    std::string log_filepath = dir_path + "/" + log_filename;
    log_file.open(log_filepath.c_str(),
                  std::ios_base::out | std::ios_base::app);
    if(!log_file.is_open())
        std::cerr << "Failed to open log file \"" << log_filepath << "\""
                  << std::endl;
}

Logger::~Logger()
{
    if(log_file.is_open()) log_file.close();
}