#include "generator.h"

bool Generator::initiated = false;
std::vector<std::string> Generator::lines;

std::string Generator::generate(uint32_t amount)
{
    if (!initiated) return std::string();
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(lines.begin(), lines.end(), g);

    std::stringstream ss;
    for (uint32_t i = 0; i < amount; ++i)
    {
        ss << lines[i] << " ";
    }
    std::string output = ss.str();

    return output.substr(0, output.length() - 1);
}

void Generator::init(std::string filepath)
{
    if (initiated) return;
    std::ifstream file(filepath);
    if (!file)
    {
        std::cerr << "Unable to open file\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    initiated = true;
}

void Generator::change_file(std::string filepath)
{
    initiated = false;
    std::ifstream file(filepath);
    if (!file)
    {
        std::cerr << "Unable to open file\n";
        return;
    }

    lines.clear();
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    initiated = true;
}
