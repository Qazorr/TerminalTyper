#include "typer.h"

enum menu_item
{
    START,
    OPTIONS,
    QUIT,
    MENU_FIRST = START,
    MENU_LAST = QUIT
};

std::string get_option_name(menu_item item)
{
    std::string option_name = ">>  <<", option;
    switch (item)
    {
    case START:
        option = "start";
        break;
    case OPTIONS:
        option = "options";
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

enum settings_item
{
    WORDS,
    FILENAME,
    SAVE,
    EXIT,
    SETTINGS_FIRST = WORDS,
    SETTINGS_LAST = EXIT
};

std::string get_settings_name(settings_item item)
{
    std::string option_name = ">  <", option;
    switch (item)
    {
    case WORDS:
        option = "number of words";
        break;
    case FILENAME:
        option = "filename";
        break;
    case SAVE:
        option = "save";
        break;
    case EXIT:
        option = "exit";
        break;
    default:
        option = "bad option";
        break;
    }
    return option_name.insert(2, option);
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

bool yes_no_question(std::string question)
{
    char in;
    std::cout << question << " (Y/N)\r";
    std::cout.flush();
    do
        in = tolower(get_input());
    while (in != 'y' && in != 'n');
    return in == 'y';
}

int16_t handle_arrow_keys(char key)
{
    switch (key)
    {
    case ARROW_UP:
        return -1;
    case ARROW_DOWN:
        return 1;
    default:
        return 0;
    }
}

void switch_menu_item(int16_t move, uint16_t &current_row, const uint16_t lower_boundary, const uint16_t upper_boundary)
{
    current_row += move;
    current_row = current_row < lower_boundary ? lower_boundary : current_row;
    current_row = current_row > upper_boundary ? upper_boundary : current_row;
}

Typer::Typer(std::string config_filename) : config_filename(config_filename)
{
    this->load_settings(this->config_filename);
    Generator::init(this->settings["words_filename"]);
    this->results = {0, 0, 0, Generator::generate(std::stoi(this->settings["no_words"]))};
}

void Typer::select_menu()
{
    std::string option_name;
    const uint16_t row_begin = 5, row_separate = 2;
    uint16_t current_row = row_begin;
    int16_t move = 0;
    char key;

    system("clear");
    while (true)
    {
        terminal_jump_to(0, 0);
        std::cout << "\033[1;34mWelcome to TerminalTyper!\n"
                  << "Use arrow UP/DOWN to move around.\n"
                  << "Press ENTER to confirm\n"
                  << RESET;
        for (int i = MENU_FIRST; i <= MENU_LAST; i++)
        {
            terminal_jump_to(row_begin + i * row_separate, 0);
            menu_item item = static_cast<menu_item>(i);
            option_name = current_row == row_begin + i * row_separate ? OPTION_PICKED_COLOR + get_option_name(item) + RESET
                                                                      : get_option_name(item);
            std::cout << option_name << std::endl;
        }
        terminal_jump_to(current_row, 2);
        std::cout.flush();
        key = get_input();
        move = handle_arrow_keys(key) * row_separate;
        if (key == ENTER)
        {
            switch ((current_row - row_begin) / row_separate)
            {
            case START:
                this->reset(Generator::generate(std::stoi(this->settings["no_words"])));
                this->start_test(true);
                break;
            case OPTIONS:
                this->change_options();
                break;
            case QUIT:
                if (this->settings_changed)
                {
                    terminal_jump_to(row_begin + (MENU_LAST + 1) * row_separate, 0);
                    if (yes_no_question("You have unsaved changes to your configuration, would u like to save?"))
                        this->save_settings(this->config_filename);
                }
                this->quit();
            default:
                break;
            }
            system("clear");
        }
        else
        {
            switch_menu_item(move, current_row, row_begin, row_begin + MENU_LAST * row_separate);
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
    terminal_jump_to(10, 0);
    if (yes_no_question("Would you like to do the same text again?"))
    {
        this->reset();
        this->start_test(show_stats, allow_jump);
    }
}

void Typer::reset(std::string &&goal)
{
    if (goal == "") // same goal as previously
        this->results = {0, 0, 0, this->results.goal};
    else // new goal
        this->results = {0, 0, 0, std::move(goal)};
}

void Typer::change_options()
{
    std::string option_name;
    const uint16_t row_begin = 5, row_separate = 2;
    uint16_t current_row = row_begin;
    int16_t move = 0;
    char key;

    system("clear");
    while (true)
    {
        terminal_jump_to(0, 0);
        std::cout << "\033[1;34mWelcome to TerminalTyper options menu!\n"
                  << "Use arrow UP/DOWN to move around.\n"
                  << "Press ENTER to choose\n"
                  << RESET;
        for (int i = SETTINGS_FIRST; i <= SETTINGS_LAST; i++)
        {
            terminal_jump_to(row_begin + i * row_separate, 0);
            settings_item item = static_cast<settings_item>(i);
            option_name = current_row == row_begin + i * row_separate ? OPTION_PICKED_COLOR + get_settings_name(item) + RESET
                                                                      : get_settings_name(item);
            std::cout << option_name << std::endl;
        }
        terminal_jump_to(current_row, 2);
        std::cout.flush();
        key = get_input();
        move = handle_arrow_keys(key) * row_separate;
        if (key == ENTER)
        {
            switch ((current_row - row_begin) / row_separate)
            {
            case WORDS:
                this->change_words_amount();
                break;
            case FILENAME:
                this->change_words_filename();
                break;
            case SAVE:
                this->save_settings(this->config_filename);
            case EXIT:
                if (this->settings_changed)
                {
                    terminal_jump_to(row_begin + (SETTINGS_LAST + 1) * row_separate, 0);
                    if (yes_no_question("You have unsaved changes to your configuration, would u like to save?"))
                        this->save_settings(this->config_filename);
                }
                return;
            default:
                break;
            }
            system("clear");
        }
        else
        {
            switch_menu_item(move, current_row, row_begin, row_begin + SETTINGS_LAST * row_separate);
        }
    }
}

void Typer::change_words_amount()
{
    std::string option_name;
    const uint16_t row_begin = 5, row_separate = 1;
    const uint16_t no_options = 10, value_jump = 10;
    const std::string element_before_option = "-> ";
    uint16_t current_row = row_begin;
    int16_t move = 0;
    char key;

    system("clear");
    while (true)
    {
        terminal_jump_to(0, 0);
        std::cout << "\033[1;34mChange amount of words displayed for you to type\n"
                  << "Use arrow UP/DOWN to move around.\n"
                  << "Press ENTER to choose words amount\n"
                  << RESET;
        for (int i = 0; i < no_options; i++)
        {
            terminal_jump_to(row_begin + i * row_separate, 0);
            option_name = current_row == row_begin + i * row_separate ? OPTION_PICKED_COLOR + element_before_option + std::to_string((i + 1) * value_jump) + RESET
                                                                      : element_before_option + std::to_string((i + 1) * value_jump);
            std::cout << option_name << std::endl;
        }
        terminal_jump_to(current_row, 2);
        std::cout.flush();
        key = get_input();
        move = handle_arrow_keys(key) * row_separate;
        if (key == ENTER)
        {
            this->settings["no_words"] = std::to_string(value_jump * (((current_row - row_begin) / row_separate) + 1));
            this->settings_changed = true;
            return;
        }
        else
            switch_menu_item(move, current_row, row_begin, row_begin + (no_options - 1) * row_separate);
    }
}

void Typer::change_words_filename()
{
    system("clear");
    terminal_jump_to(0, 0);
    std::cout << "\033[1;34mChange words input filename.\n"
              << "Remember the file must contain 1 word per line\n"
              << RESET;

    terminal_jump_to(5, 0);
    std::string filename;
    std::cout << "Enter full filename with extension: ";
    std::cin >> filename;
    this->settings["words_filename"] = filename;
    this->settings_changed = true;
}

void Typer::run()
{
    this->select_menu();
}