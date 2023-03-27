#include "typer.h"

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

Typer::Typer(std::string goal)
{
    results = {0, 0, 0, goal};
};

void Typer::start_test(bool show_stats, bool allow_jump)
{
    char in;
    auto begin = std::chrono::steady_clock::now();
    system("clear");
    while (this->results.user_score != this->results.goal.length())
    {
        if(show_stats) this->results.time = since(begin).count();
        this->display_progress(show_stats);
        if(allow_jump) {
            terminal_jump_to(0, this->results.user_score);
            std::cout.flush();
        }
        in = get_input();
        if (in == this->results.goal.at(this->results.user_score))
            (this->results.user_score)++;
        (this->results.input_count)++;
    }
    this->results.time = since(begin).count();
    this->display_finish();
}

void Typer::reset(std::string&& goal)
{
    // perform reset (the same goal from beginning)
    if(goal == "") {
        this->results = {0, 0, 0, this->results.goal};
    } else {
        this->results = {0, 0, 0, std::move(goal)};
    }
    terminal_jump_to(10, 0);
    std::cout << "\nWaiting for key press...\r";
    std::cout.flush();
    get_input();
}
