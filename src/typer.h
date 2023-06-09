#pragma once

#include "generator.h"
#include "logger.h"

#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <chrono>
#include <iomanip>
#include <math.h>
#include <cctype>
#include <map>
#include <filesystem>
#include <sys/ioctl.h>

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
#define DESCRIPTION_COLOR "\033[1;34m"
#define OPTION_CONFIG_COLOR "\033[1;3;4;35m"
#define OPTION_PICKED_COLOR "\033[1;4;6;32m"
#define RESET "\033[0m"

#define DEFAULT_CONFIG_FILENAME "config.txt"

#define CLASSIC_MODE "0"
#define TEXT_MODE "1"

struct option
{
    std::string name;
    std::string value;
};

struct terminal_size
{
    int width, height;

    bool operator==(const terminal_size &other) const
    {
        return width == other.width && height == other.height;
    }

    bool operator!=(const terminal_size &other) const
    {
        return !(*this == other);
    }
};

struct test_result
{
    int64_t time;
    uint32_t user_score;
    uint32_t input_count;
    std::string goal;
};

/// @brief get one char input from stdin without the need of pressing enter
/// @return character clicked
char get_input();

/// @brief ask user a yes/no question
/// @param question question to be asked
/// @return true if user chose 'yes', false if 'no' was chosen
bool yes_no_question(std::string question);

/// @brief translate arrow key pressed to movement value
/// @param key key pressed by user
/// @return 1 for arrow down, -1 for arrow up, else 0
int16_t handle_up_down_arrow_key(char key);

/// @brief translate arrow key pressed to movement value
/// @param key key pressed by user
/// @return 1 for arrow right, -1 for arrow left, else 0
int16_t handle_left_right_arrow_key(char key);

/// @brief safely change the value of current position without exiting the boundaries in one direction (horizontally/vertically)
/// @param move 1 for up/right, -1 for down/left movement
/// @param current_pos x/y position
/// @param lower_boundary smallest possible value
/// @param upper_boundary biggest possible value
void switch_menu_item(int16_t move, uint16_t &current_pos, const uint16_t lower_boundary, const uint16_t upper_boundary);

/// @brief clear terminal output
void clear_terminal();

terminal_size get_terminal_size();

void print_centered(const std::string &text, const int desired_size = 30, const char begin_end_char = '|');

class Typer
{
private:
    test_result results;
    std::string config_filename;
    std::map<std::string, std::string> settings;
    bool settings_changed = false;
    Logger logger, results_logger;

    /// @param start relative time point
    /// @return time from the start point to now in milliseconds
    template <
        class result_t = std::chrono::milliseconds,
        class clock_t = std::chrono::steady_clock,
        class duration_t = std::chrono::milliseconds>
    std::chrono::milliseconds since(std::chrono::time_point<clock_t, duration_t> const &start)
    {
        return std::chrono::duration_cast<result_t>(clock_t::now() - start);
    }

    /// @brief split target goal of current test into two parts (already typed and to be typed)
    /// @param left already typed by user
    /// @param right to be typed by user
    void split_goal_to(std::string &left, std::string &right)
    {
        left = this->results.goal.substr(0, this->results.user_score);
        right = this->results.goal.substr(this->results.user_score);
    }

    /// @brief format given float number to a given precission
    /// @param f number to be formatted
    /// @param digits number of digits to be rounded to
    /// @return formatted float number
    std::string format(float f, int digits)
    {
        std::ostringstream ss;
        ss.precision(digits);
        ss << f;
        return ss.str();
    }

    /// @return current test accuracy
    float get_accuracy() { return this->results.input_count == 0 ? 0.f : (float)this->results.user_score / this->results.input_count; }

    /// @return current test time in seconds
    float get_time_s() { return this->results.time / 1000.f; }

    /// @return current test WPM
    float get_WPM() { return (this->results.user_score / 5.f) / (this->results.time / 60000.f); }

    /// @return current test characters amount
    uint16_t get_characters_amount() { return this->results.goal.length(); }

    /// @return current test word count
    uint16_t get_words_amount()
    {
        std::stringstream ss(this->results.goal);
        std::string word;
        uint16_t count = 0;
        while (ss >> word)
            ++count;
        return count;
    }

    /// @brief display test stats (accuracy, time, WPM)
    void display_stats()
    {
        const int desired_width = 30;
        const std::string bottom_top_line(desired_width, '-');
        terminal_jump_to(STATS_START_ROW, STATS_START_COL);
        std::cout << STATS_COLOR;
        print_centered(bottom_top_line, desired_width, '+');
        print_centered("Accuracy: " + this->format(this->get_accuracy() * 100, 4) + "% ", desired_width);
        print_centered("Elapsed = " + this->format(this->results.time / 1000.f, 4) + "s ", desired_width);
        print_centered("WPM = " + this->format(this->get_WPM(), 4) + " ", desired_width);
        print_centered("Characters:  " + std::to_string(this->results.user_score) + "/" + std::to_string(this->results.goal.length()) + " ", desired_width);
        print_centered(bottom_top_line, desired_width, '+');
        std::cout << RESET << "\n";
    }

    /// @brief display test progress (already typed and to be typed)
    void display_progress()
    {
        std::string left, right;
        split_goal_to(left, right);
        terminal_jump_to(TEXT_START_ROW, TEXT_START_COL);
        std::cout << "\r" << CORRECT_COLOR << left << INITIAL_COLOR << right << RESET << "\n";
        if (this->settings["show_stats"] == "1")
            this->display_stats();
    }

    /// @brief display finished test stats
    void display_finish()
    {
        terminal_jump_to(TEXT_START_ROW, TEXT_START_COL);
        std::cout << "\r" << FINISHED_COLOR << this->results.goal << RESET << std::endl;
        display_stats();
    }

    /// @brief quit app
    void quit()
    {
        clear_terminal();
        terminal_jump_to(0, 0);
        this->logger << "app quit";
        exit(EXIT_SUCCESS);
    }

    /// @brief load defined default settings into the app
    void load_default_settings()
    {
        this->settings = {
            {"mode", "0"},
            {"no_words", "10"},
            {"words_filename", "words/words.txt"},
            {"trailing_cursor", "1"},
            {"show_stats", "1"}};
        this->logger << "loaded default settings";
    }

    /// @brief load settings from app's config file
    void load_settings()
    {
        std::ifstream infile(this->config_filename);
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
            this->logger << "loaded settings from config file " + this->config_filename;
        }
        else
            save_settings(true);
    }

    /// @brief save current app settings
    /// @param default_settings if true restore default settings (defaults to false)
    void save_settings(bool default_settings = false)
    {
        if (default_settings)
            this->load_default_settings();
        std::ofstream outfile(this->config_filename);
        for (auto const &[name, value] : settings)
            outfile << name << "=" << value << std::endl;
        outfile.close();
        this->settings_changed = false;
        this->logger << "settings saved to " + this->config_filename;
    }

    /// @brief menu for options with swichable values (e.g. true/false)
    /// @param setting_name what appears on the option menu
    /// @param option_name_value possible names and values for given option like {{"ON", "1"}, {"OFF", "0"}}
    void change_switch_option(std::string setting_name, std::vector<option> option_name_value)
    {
        std::string option_name;

        std::stringstream ss;
        ss << DESCRIPTION_COLOR << "Change " << setting_name << " value.\n"
           << "Note " << OPTION_CONFIG_COLOR << "this color" << RESET << DESCRIPTION_COLOR " means this setting is already chosen\n"
           << "Use arrow LEFT/RIGHT to move around.\n"
           << "Press ENTER to choose a value\n"
           << "Press 'q' to cancel\n"
           << RESET;
        std::string description = ss.str();

        const int16_t row_begin = std::count(description.begin(), description.end(), '\n') + 2, col_begin = 5, col_separate = 10;
        uint16_t current_col = col_begin;
        int16_t move = 0;
        char key;

        auto is_hovered = [&current_col](uint32_t option_number)
        { return (col_begin + option_number * col_separate) == current_col; };

        clear_terminal();
        while (true)
        {
            terminal_jump_to(0, 0);
            std::cout << description;
            for (uint16_t i = 0; i < option_name_value.size(); i++)
            {
                terminal_jump_to(row_begin, col_begin + i * col_separate);
                if (this->is_from_config(option_name_value[i].value, setting_name) && !is_hovered(i))
                    std::cout << OPTION_CONFIG_COLOR << option_name_value[i].name << RESET;
                else
                {
                    option_name = is_hovered(i) ? OPTION_PICKED_COLOR + option_name_value[i].name + RESET
                                                : option_name_value[i].name;
                    std::cout << option_name;
                }
            }
            terminal_jump_to(row_begin, current_col);
            std::cout.flush();
            key = get_input();
            move = handle_left_right_arrow_key(key) * col_separate;
            if (key == ENTER)
            {
                this->settings[setting_name] = option_name_value[((current_col - col_begin) / col_separate)].value;
                this->settings_changed = true;
                this->logger << setting_name + " changed to: < " + option_name_value[((current_col - col_begin) / col_separate)].name + " >";
                return;
            }
            else if (key == GO_BACK_SHORTCUT)
                return;
            else
                switch_menu_item(move, current_col, col_begin, col_begin + (option_name_value.size() - 1) * col_separate);
        }
    }

    bool is_from_config(const std::string &option_value, const std::string &setting_name)
    {
        return option_value == this->settings[setting_name];
    }

    /// @brief get all files from given path
    /// @param path path to directory with files
    /// @return vector with file names without path
    std::vector<std::string> get_filenames_vector(std::string path)
    {
        std::vector<std::string> files;
        for (const auto &entry : std::filesystem::directory_iterator(path))
            files.push_back(entry.path().filename());
        return files;
    }

    /// @brief get first file from given path
    /// @param path path to directory with files
    /// @return name of first file without the path to it if any exists, else ""
    std::string get_first_file(std::string path)
    {
        try
        {
            return (*std::filesystem::directory_iterator(path)).path().filename();
        }
        catch (const std::exception &e)
        {
            return std::string("");
        }
    }

public:
    /// @brief create Typer app instance with default config file (DEFAULT_CONFIG_FILENAME) or creates one if file with the name doesn't exist
    Typer();

    /// @brief create Typer app instance
    /// @param config_filename file from which settings are loaded (creates the file if it doesn't exist)
    Typer(std::string config_filename);

    /// @brief main app menu
    void select_menu();

    /// @brief begin typing test with defined goal
    void start_test();

    /// @brief change the test goal or reset the goal to be the same as the previous one
    /// @param goal test text (defaults to ""), when "" resets the goal
    void reset(std::string &&goal = "");

    /// @brief settings menu
    void change_settings();

    /// @brief menu for words amount option
    void change_words_amount();

    /// @brief menu for choosing filename for goal generation
    /// @param path path to directory with files
    void change_words_filename(const std::string &path = "words");

    /// @brief run main app menu
    void run();
};