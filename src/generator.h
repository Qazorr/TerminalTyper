#pragma once

#include "logger.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

class Generator {
  private:
    static std::vector<std::string> lines;
    static bool initiated;
    static Logger logger;

  public:
    Generator() = delete;

    static void init(std::string filepath = "txt/words.txt");
    static void change_file(std::string filepath);
    static std::string generate(uint32_t amount);
    static std::string get_text(std::string filepath);
};