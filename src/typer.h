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
#include <filesystem>

#define terminal_jump_to(row, col) printf("\033[%d;%dH", row, col);
#define TEXT_START_ROW 0
#define TEXT_START_COL 0
#define STATS_START_ROW 5
#define STATS_START_COL 0

#define GO_BACK_SHORTCUT 113 // q

#define ENTER 10
#define ESCAPE 27
#define ARROW_UP 65
#define ARROW_DOWN 66
#define ARROW_RIGHT 67
#define ARROW_LEFT 68

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
int16_t handle_up_down_arrow_key(char key);
int16_t handle_left_right_arrow_key(char key);

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

    void display_progress()
    {
        std::string left, right;
        split_goal_to(left, right);
        terminal_jump_to(TEXT_START_ROW, TEXT_START_COL);
        std::cout << "\r" << CORRECT_COLOR << left << INITIAL_COLOR << right << RESET << "\n";
        if (this->settings["show_stats"] == "1")
            this->display_stats();
    }

    void display_finish()
    {
        terminal_jump_to(TEXT_START_ROW, TEXT_START_COL);
        std::cout << "\r" << FINISHED_COLOR << this->results.goal << RESET << std::endl;
        display_stats();
    }

    void quit()
    {
        system("clear");
        terminal_jump_to(0, 0);
        exit(EXIT_SUCCESS);
    }

    void load_default_settings()
    {
        this->settings = {
            {"no_words", "10"},
            {"words_filename", "txt/words.txt"},
            {"trailing_cursor", "1"},
            {"show_stats", "1"}};
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
        if (default_settings)
            this->load_default_settings();
        std::ofstream outfile(config_filename);
        for (auto const &[name, value] : settings)
            outfile << name << "=" << value << std::endl;
        outfile.close();
        this->settings_changed = false;
    }

    void change_switch_option(std::string option);

public:
    Typer(std::string config_filename = DEFAULT_CONFIG_FILENAME);

    void select_menu();
    void start_test();
    void reset(std::string &&goal = "");
    void change_settings();
    void change_words_amount();
    void change_words_filename();
    void run();
};