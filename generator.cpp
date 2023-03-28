#include "generator.h"

std::string Generator::generate(uint32_t amount, std::string filepath)
{
    std::ifstream file(filepath);
    if (!file)
    {
        std::cerr << "Unable to open file\n";
        return std::string();
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(lines.begin(), lines.end(), g);

    std::string output = "";

    for (int i = 0; i < amount; ++i)
        output += lines[i] + " ";

    return output.substr(0, output.length() - 1);
}