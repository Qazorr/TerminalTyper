#include "generator.h"

bool Generator::initiated = false;
std::vector<std::string> Generator::lines{};

std::string Generator::generate(uint32_t amount)
{
    if (!is_initiated())
        return std::string();
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(lines.begin(), lines.end(), g);

    std::string output = "";
    for (int i = 0; i < amount; ++i)
        output += lines[i] + " ";

    return output.substr(0, output.length() - 1);
}

void Generator::init(std::string filename)
{
    if (is_initiated())
        return;
    std::ifstream file(filename);
    if (!file)
    {
        std::cerr << "Unable to open file\n";
        return;
    }

    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    std::cout << lines.size() << std::endl;
    initiated = true;
}