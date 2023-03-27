#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <chrono>

#define move(row, col) printf("\033[%d;%dH", row, col);

#ifdef _WIN_32
char getch();
#else
char getch()
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
#endif

struct test_result
{
    int64_t time;
    uint32_t user_score;
    std::string goal;
};


void split(uint32_t pos, const std::string &str, std::string &left, std::string &right)
{
    left = str.substr(0, pos);
    right = str.substr(pos);
}

void display_progress(test_result result)
{
    std::string left, right;
    split(result.user_score, result.goal, left, right);
    std::cout << "\r\033[1;32m" << left << "\033[0m" << right << "\n";
    move(5, 0);
    std::cout << "\nElapsed(ms) = " << result.time << std::endl;
}

void display_finish(test_result result)
{
    system("clear");
    std::cout << "\r\033[1;34m" << result.goal << "\033[0m" << std::endl;
    std::cout << "Accuracy: " << ((float)result.goal.length() / result.user_score) * 100 << "%" << std::endl;
    std::cout << "Elapsed time(ms) = " << result.time << std::endl;
}

template <
    class result_t = std::chrono::milliseconds,
    class clock_t = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds>
auto since(std::chrono::time_point<clock_t, duration_t> const &start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

test_result start_test(std::string goal)
{
    std::string user = "";
    char in;
    uint32_t user_progress = 0, no_inputs = 0;
    auto begin = std::chrono::steady_clock::now();
    system("clear");
    while (user_progress != goal.length())
    {
        display_progress({since(begin).count(), user_progress, goal});
        move(0, user_progress);
        in = getch();
        if (in == goal.at(user_progress))
        {
            user += in;
            ++user_progress;
        }
        ++no_inputs;
    }
    return {since(begin).count(), no_inputs, goal};
}

int main()
{
    std::string goal = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
    auto score = start_test(goal);
    display_finish(score);
}