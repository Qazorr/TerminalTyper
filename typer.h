#pragma once

#include "generator.h"

#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <chrono>
#include <iomanip>
#include <math.h>
#include <cctype>

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

struct test_result
{
    int64_t time;
    uint32_t user_score;
    uint32_t input_count;
    std::string goal;
};

char get_input();

class Typer
{
private:
    test_result results;
    uint16_t no_words;
    std::string words_filename;

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

    void ask_for_repeat(bool show_stats, bool allow_jump)
    {
        terminal_jump_to(10, 0);
        char in;
        std::cout << "Would you like to do the same text again? (Y/N)\r";
        std::cout.flush();
        do
        {
            in = tolower(get_input());
        } while (in != 'y' && in != 'n');
        if (in == 'y')
        {
            this->reset();
            this->start_test(show_stats, allow_jump);
        }
        else
        {
            std::string new_goal = Generator::generate(this->no_words);
            this->reset(std::move(new_goal));
        }
    }

public:
    Typer(uint16_t default_no_words = 10, std::string words_filename = "words.txt");

    void select_menu();
    void start_test(bool show_stats = false, bool allow_jump = true);
    void reset(std::string &&goal = "");
    void run();
};