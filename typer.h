#pragma once

#include "generator.h"

#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <chrono>
#include <iomanip>
#include <math.h>
#include <cctype>
#include <map>

#define terminal_jump_to(row, col) printf("\033[%d;%dH", row, col);
#define TEXT_START_ROW 0
#define TEXT_START_COL 0
#define STATS_START_ROW 5
#define STATS_START_COL 0

#define ENTER 10
#define ARROW_UP 65
#define ARROW_DOWN 66

#define INITIAL_COLOR "\033[0m"
#define CORRECT_COLOR "\033[1;32m"
#define FINISHED_COLOR "\033[1;34m"
#define STATS_COLOR "\033[1;36m"
#define OPTION_PICKED_COLOR "\033[1;4;6;32m"
#define RESET "\033[0m"

#define DEFAULT_CONFIG_FILENAME "config.txt"

struct test_result
{
    int64_t time;
    uint32_t user_score;
    uint32_t input_count;
    std::string goal;
};

char get_input();
bool yes_no_question(std::string question);
class Typer
{
private:
    test_result results;
    std::string config_filename;
    std::map<std::string, std::string> settings;
    bool settings_changed = false;

    template <
        class result_t = std::chrono::milliseconds,
        class clock_t = std::chrono::steady_clock,
        class duration_t = std::chrono::milliseconds>
    std::chrono::milliseconds since(std::chrono::time_point<clock_t, duration_t> const &start)
    {
        return std::chrono::duration_cast<result_t>(clock_t::now() - start);
    }

    void split_goal_to(std::string &left, std::string &right)
    {
        left = this->results.goal.substr(0, this->results.user_score);
        right = this->results.goal.substr(this->results.user_score);
    }

    std::string format(float f, int digits)
    {
        std::ostringstream ss;
        ss.precision(digits);
        ss << f;
        return ss.str();
    }

    float get_accuracy() { return this->results.input_count == 0 ? 0.f : (float)this->results.user_score / this->results.input_count; }
    float get_time_s() { return this->results.time / 1000.f; }
    float get_WPM() { return (this->results.user_score / 5.f) / (this->results.time / 60000.f); }

    void display_stats()
    {
        terminal_jump_to(STATS_START_ROW, STATS_START_COL);
        std::cout << STATS_COLOR;
        std::cout << '+' << std::string(25, '-') << '+' << std::endl;
        std::cout << '|' << std::setw(25) << "Accuracy: " + this->format(this->get_accuracy() * 100, 4) + "% " << '|' << std::endl;
        std::cout << '|' << std::setw(25) << "Elapsed = " + this->format(this->results.time / 1000.f, 4) + "s " << '|' << std::endl;
        std::cout << '|' << std::setw(25) << "WPM = " + this->format(this->get_WPM(), 4) + " " << '|' << std::endl;
        std::cout << '+' << std::string(25, '-') << RESET << "+\n\n";
    }

    void display_progress(bool show_stats = false)
    {
        std::string left, right;
        split_goal_to(left, right);
        terminal_jump_to(TEXT_START_ROW, TEXT_START_COL);
        std::cout << "\r" << CORRECT_COLOR << left << INITIAL_COLOR << right << RESET << "\n";
        if (show_stats)
            this->display_stats();
    }

    void display_finish()
    {
        terminal_jump_to(TEXT_START_ROW, TEXT_START_COL);
        std::cout << "\r" << FINISHED_COLOR << this->results.goal << RESET << std::endl;
        display_stats();
    }

    void restart()
    {
        system("clear");
        terminal_jump_to(0, 0);
        std::cout << "NOT IMPLEMENTED\n";
        exit(EXIT_FAILURE);
    }

    void quit()
    {
        system("clear");
        terminal_jump_to(0, 0);
        exit(EXIT_SUCCESS);
    }

    void load_settings(std::string config_filename)
    {
        std::ifstream infile(config_filename);
        if (infile.is_open())
        {
            std::string line, name, value;
            while (std::getline(infile, line))
            {
                std::size_t pos = line.find('=');
                std::string name = line.substr(0, pos), value = line.substr(pos + 1);

                settings[name] = value;
            }
            infile.close();
        }
        else
            save_settings(config_filename, true);
    }

    void save_settings(std::string config_filename, bool default_settings = false)
    {
        if(default_settings) {
            this->settings["no_words"] = "10";
            this->settings["words_filename"] = "words.txt";
        }
        std::ofstream outfile(config_filename);
        for (auto const &[name, value] : settings)
            outfile << name << "=" << value << std::endl;
        outfile.close();
        this->settings_changed = false;
    }

public:
    Typer(std::string config_filename = DEFAULT_CONFIG_FILENAME);

    void select_menu();
    void start_test(bool show_stats = false, bool allow_jump = true);
    void reset(std::string &&goal = "");
    void change_options();
    void change_words_amount();
    void change_words_filename();
    void run();
};