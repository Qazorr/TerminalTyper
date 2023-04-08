#include "generator.h"

bool Generator::initiated = false;
std::vector<std::string> Generator::lines;
Logger Generator::logger("generator.log", "generator.cpp");

std::string Generator::generate(uint32_t amount)
{
    if (!initiated)
        return std::string();
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(lines.begin(), lines.end(), g);

    std::stringstream ss;
    for (uint32_t i = 0; i < amount; ++i)
        ss << lines[i] << " ";
    std::string output = ss.str();

    logger << "generated " + std::to_string(amount) + " words";

    return output.substr(0, output.length() - 1);
}

std::string Generator::get_text(std::string filepath)
{
    std::ifstream file(filepath);
    if (!file)
    {
        logger << "=ERROR= Unable to open file " + filepath;
        return std::string();
    }
    std::string line;
    std::getline(file, line);
    logger << "text from " + filepath + " obtained";
    return line;
}

void Generator::init(std::string filepath)
{
    if (initiated)
        return;
    std::ifstream file(filepath);
    if (!file)
    {
        logger << "=ERROR= Unable to open file " + filepath;
        return;
    }

    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    initiated = true;
    logger << "generator initiated with file " + filepath;
}

void Generator::change_file(std::string filepath)
{
    initiated = false;
    std::ifstream file(filepath);
    if (!file)
    {
        logger << "=ERROR= Unable to open file " + filepath;
        return;
    }

    lines.clear();
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    initiated = true;
    logger << "generator initiated with file " + filepath;
}
