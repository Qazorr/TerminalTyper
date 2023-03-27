#pragma once

#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <chrono>
#include <iomanip>
#include <math.h>

#define terminal_jump_to(row, col) printf("\033[%d;%dH", row, col);
#define TEXT_START_ROW 0
#define TEXT_START_COL 0
#define STATS_START_ROW 5
#define STATS_START_COL 0

#define INITIAL_COLOR "\033[0m"
#define CORRECT_COLOR "\033[1;32m"
#define FINISHED_COLOR "\033[1;34m"
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
        std::cout << '+' << std::string(25, '-') << '+' << std::endl;
        std::cout << '|' << std::setw(25) << "Accuracy: " + this->format(this->get_accuracy() * 100, 4) + "% " << '|' << std::endl;
        std::cout << '|' << std::setw(25) << "Elapsed = " + this->format(this->results.time / 1000.f, 4) + "s " << '|' << std::endl;
        std::cout << '|' << std::setw(25) << "WPM = " + this->format(this->get_WPM(), 4) + " " << '|' << std::endl;
        std::cout << '+' << std::string(25, '-') << "+\n\n";
    }

    void display_progress(bool show_stats = false)
    {
        std::string left, right;
        split_goal_to(left, right);
        terminal_jump_to(TEXT_START_ROW, TEXT_START_COL);
        std::cout << "\r" << CORRECT_COLOR << left << INITIAL_COLOR << right << RESET << "\n";
        if (show_stats) this->display_stats();
    }

    void display_finish()
    {
        terminal_jump_to(TEXT_START_ROW, TEXT_START_COL);
        std::cout << "\r" << FINISHED_COLOR << this->results.goal << RESET << std::endl;
        display_stats();
    }

public:
    Typer() = delete;
    Typer(std::string goal);

    void start_test(bool show_stats = false, bool allow_jump = true);
    void reset(std::string &&goal = "");
};