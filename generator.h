#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <algorithm>

class Generator
{
public:
    Generator() = delete;

    static std::string generate(uint32_t amount, std::string filepath="words.txt");
};
