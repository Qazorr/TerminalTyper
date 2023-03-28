#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <algorithm>

class Generator
{
private:
    static std::vector<std::string> lines;
    static bool initiated;

public:
    Generator() = delete;

    static void init(std::string filename = "words.txt");
    static inline bool is_initiated() { return initiated; }
    static std::string generate(uint32_t amount);
};