#include "typer.h"

enum menu_item
{
    START,
    QUIT
};

std::string get_option_name(menu_item item)
{
    std::string option_name = ">>  <<", option;
    switch (item)
    {
    case START:
        option = "start";
        break;
    case QUIT:
        option = "quit";
        break;
    default:
        option = "bad option";
        break;
    }
    return option_name.insert(3, option);
}

char get_input()
{
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

int16_t handle_arrow_keys(char key)
{
    switch (key)
    {
    case ENTER:
        return -1;
    case ARROW_UP:
        return -2;
    case ARROW_DOWN:
        return 2;
    default:
        return 0;
    }
}

void switch_menu_item(int16_t move, uint16_t &current_row, const uint16_t lower_boundary, const uint16_t upper_boundary)
{
    current_row += move;
    if (current_row < lower_boundary)
        current_row = lower_boundary;
    if (current_row > upper_boundary)
        current_row = upper_boundary;
}

Typer::Typer(uint16_t default_no_words, std::string words_filename) : no_words(default_no_words), words_filename(words_filename)
{
    this->results = {0, 0, 0, Generator::generate(this->no_words, this->words_filename)};
}

void Typer::select_menu()
{
    std::string option_name;
    const uint16_t row_begin = 5;
    uint16_t current_row = row_begin;
    int16_t move = 0;
    char key;

    system("clear");
    while (true)
    {
        terminal_jump_to(0, 0);
        std::cout << "\033[1;34mWelcome to TerminalTyper!\nUse arrow UP/DOWN to move around.\nPress ENTER to confirm\n"
                  << RESET;
        for (int i = 0; i <= QUIT; i++)
        {
            terminal_jump_to(row_begin + i * 2, 0);
            menu_item item = static_cast<menu_item>(i);
            option_name = current_row == row_begin + i * 2 ? OPTION_PICKED_COLOR + get_option_name(item) + RESET : get_option_name(item);
            std::cout << option_name << std::endl;
        }
        terminal_jump_to(current_row, 2);
        std::cout.flush();
        key = get_input();
        move = handle_arrow_keys(key);
        if (move == -1)
        { // ENTER PRESSED
            switch ((current_row - row_begin) / 2)
            {
            case START:
                this->start_test(true);
                break;
            case QUIT:
                this->quit();
                break;
            default:
                break;
            }
            system("clear");
        }
        else
        {
            switch_menu_item(move, current_row, row_begin, row_begin + QUIT * 2);
        }
    }
}

void Typer::start_test(bool show_stats, bool allow_jump)
{
    char in;
    auto begin = std::chrono::steady_clock::now();
    std::string debug = "";
    bool started = false;

    system("clear");
    while (this->results.user_score != this->results.goal.length())
    {
        if (show_stats)
            this->results.time = since(begin).count();
        this->display_progress(show_stats);
        if (allow_jump)
        {
            terminal_jump_to(0, this->results.user_score + 1);
            std::cout.flush();
        }
        in = get_input();
        if (!started)
        {
            started = true;
            begin = std::chrono::steady_clock::now();
        }
        if (in == this->results.goal.at(this->results.user_score))
            (this->results.user_score)++;
        debug += in;
        (this->results.input_count)++;
    }
    this->results.time = since(begin).count();
    this->display_finish();
    this->ask_for_repeat(show_stats, allow_jump);
}

void Typer::reset(std::string &&goal)
{
    system("clear");
    if (goal == "")
    { // same goal as previously
        this->results = {0, 0, 0, this->results.goal};
    }
    else
    { // new goal
        this->results = {0, 0, 0, std::move(goal)};
    }
}
