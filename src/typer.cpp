#include "typer.h"

enum menu_item { START, OPTIONS, QUIT, MENU_FIRST = START, MENU_LAST = QUIT };

std::string get_option_name(menu_item item)
{
    std::string option_name = ">>  <<", option;
    switch(item) {
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

enum settings_item {
    MODE,
    WORDS,
    FILENAME,
    TRAILING_CURSOR,
    SHOW_STATS,
    RESTORE_DEFAULT,
    SAVE,
    EXIT,
    SETTINGS_FIRST = MODE,
    SETTINGS_LAST  = EXIT
};

std::string get_settings_name(settings_item item)
{
    std::string option_name = ">  <", option;
    switch(item) {
        case MODE:
            option = "typer mode";
            break;
        case WORDS:
            option = "number of words";
            break;
        case FILENAME:
            option = "filename";
            break;
        case TRAILING_CURSOR:
            option = "trailing cursor";
            break;
        case SHOW_STATS:
            option = "show stats when typing";
            break;
        case RESTORE_DEFAULT:
            option = "restore settings to default";
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
    char buf           = 0;
    struct termios old = {0};
    if(tcgetattr(0, &old) < 0) perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN]  = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0) perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0) perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0) perror("tcsetattr ~ICANON");
    return (buf);
}

bool yes_no_question(std::string question)
{
    char in;
    std::cout << question << " (Y/N)\r";
    std::cout.flush();
    do
        in = tolower(get_input());
    while(in != 'y' && in != 'n');
    return in == 'y';
}

int16_t handle_up_down_arrow_key(char key)
{
    switch(key) {
        case ARROW_UP:
            return -1;
        case ARROW_DOWN:
            return 1;
        default:
            return 0;
    }
}

int16_t handle_left_right_arrow_key(char key)
{
    switch(key) {
        case ARROW_LEFT:
            return -1;
        case ARROW_RIGHT:
            return 1;
        default:
            return 0;
    }
}

void switch_menu_item(int16_t move, uint16_t &current_pos,
                      const uint16_t lower_boundary,
                      const uint16_t upper_boundary)
{
    current_pos += move;
    current_pos = current_pos < lower_boundary ? upper_boundary : current_pos;
    current_pos = current_pos > upper_boundary ? lower_boundary : current_pos;
}

void clear_terminal()
{
    system("clear");
}

terminal_size get_terminal_size()
{
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    int width  = size.ws_col;
    int height = size.ws_row;
    return {width, height};
}

void print_centered(const std::string &text, const int desired_size,
                    const char begin_end_char)
{
    int left_padding = (get_terminal_size().width - desired_size) / 2;
    std::cout << std::string(left_padding - 1, ' ') << begin_end_char
              << std::setw(desired_size) << text << begin_end_char << std::endl;
}

Typer::Typer() : Typer(DEFAULT_CONFIG_FILENAME) {}

Typer::Typer(std::string config_filename)
    : config_filename(config_filename), logger("typer.log", "typer.cpp"),
      results_logger("results.log", "typer.cpp")
{
    this->load_settings();
    Generator::init(this->settings["words_filename"]);
}

void Typer::select_menu()
{
    std::string option_name, test;
    const uint16_t row_begin = 6, row_separate = 2;
    uint16_t current_row = row_begin;
    int16_t move         = 0;
    char key;

    clear_terminal();
    while(true) {
        terminal_jump_to(0, 0);
        std::cout << "\033[1;34mWelcome to TerminalTyper!\n"
                  << "Use arrow 'UP'/'DOWN' to move around.\n"
                  << "Press 'ENTER' to confirm.\n"
                  << "You can press 'q' to quit and 'ESC' to interrupt the "
                     "already begun test.\n"
                  << RESET;
        for(int i = MENU_FIRST; i <= MENU_LAST; i++) {
            terminal_jump_to(row_begin + i * row_separate, 0);
            menu_item item = static_cast<menu_item>(i);
            option_name =
                current_row == row_begin + i * row_separate
                    ? OPTION_PICKED_COLOR + get_option_name(item) + RESET
                    : get_option_name(item);
            std::cout << option_name << std::endl;
        }
        terminal_jump_to(current_row, 2);
        std::cout.flush();
        key  = get_input();
        move = handle_up_down_arrow_key(key) * row_separate;
        if(key == ENTER) {
            switch((current_row - row_begin) / row_separate) {
                case START:
                    if(this->settings["mode"] == CLASSIC_MODE)
                        this->reset(Generator::generate(
                            std::stoi(this->settings["no_words"])));
                    else if(this->settings["mode"] == TEXT_MODE)
                        this->reset(Generator::get_text(
                            this->settings["words_filename"]));
                    else
                        return;
                    this->start_test();
                    break;
                case OPTIONS:
                    this->change_settings();
                    break;
                case QUIT:
                    if(this->settings_changed) {
                        terminal_jump_to(
                            row_begin + (MENU_LAST + 1) * row_separate, 0);
                        if(yes_no_question(
                               "You have unsaved changes to your "
                               "configuration, would u like to save?"))
                            this->save_settings();
                    }
                    this->quit();
                default:
                    break;
            }
            clear_terminal();
        }
        else if(key == GO_BACK_SHORTCUT) {
            if(this->settings_changed) {
                terminal_jump_to(row_begin + (MENU_LAST + 1) * row_separate, 0);
                if(yes_no_question("You have unsaved changes to your "
                                   "configuration, would u like to save?"))
                    this->save_settings();
            }
            this->quit();
        }
        else
            switch_menu_item(move, current_row, row_begin,
                             row_begin + MENU_LAST * row_separate);
    }
}

void Typer::start_test()
{
    char in;
    auto begin              = std::chrono::steady_clock::now();
    std::string debug       = "";
    bool started            = false;
    terminal_size term_size = get_terminal_size(), previous_term_size;

    clear_terminal();
    this->logger << "test with " + std::to_string(this->get_words_amount()) +
                        " words and " +
                        std::to_string(this->get_characters_amount()) +
                        " characters started";
    while(this->results.user_score != this->results.goal.length()) {
        previous_term_size = term_size;
        term_size          = get_terminal_size();
        if(previous_term_size != term_size) clear_terminal();
        if(this->settings["show_stats"] == "1")
            this->results.time = since(begin).count();
        this->display_progress();
        if(this->settings["trailing_cursor"] == "1") {
            terminal_jump_to(
                ((this->results.user_score + 1) / term_size.width) + 1,
                (this->results.user_score + 1) % term_size.width);
            std::cout.flush();
        }
        in = get_input();
        if(in == ESCAPE) break;
        if(!started) {
            started = true;
            begin   = std::chrono::steady_clock::now();
        }
        if(in == this->results.goal.at(this->results.user_score))
            (this->results.user_score)++;
        debug += in;
        (this->results.input_count)++;
    }
    this->results.time = since(begin).count();
    this->display_finish();

    std::stringstream ss;
    ss << std::setw(10) << std::left
       << this->format(this->get_accuracy() * 100, 4) + "%" << std::setw(10)
       << std::left << this->format(this->results.time / 1000.f, 4) + "s"
       << std::setw(10) << std::left
       << this->format(this->get_WPM(), 4) + "WPM";
    this->results_logger << ss.str();

    //? call for next user action
    terminal_jump_to(12, 0);
    std::cout << "What do you want to do next? [click first letter]"
              << " (Again/Restart/Quit)\r";
    std::cout.flush();
    do
        in = tolower(get_input());
    while(in != 'a' && in != 'r' && in != 'q');
    if(in == 'q')
        return;
    else if(in == 'a')
        this->reset();
    else {
        if(this->settings["mode"] == CLASSIC_MODE)
            this->reset(
                Generator::generate(std::stoi(this->settings["no_words"])));
        else if(this->settings["mode"] == TEXT_MODE)
            this->reset(Generator::get_text(this->settings["words_filename"]));
    }
    this->start_test();
}

void Typer::reset(std::string &&goal)
{
    if(goal == "") {
        this->results = {0, 0, 0, this->results.goal};
        this->logger << "goal reset";
    }
    else {
        this->results = {0, 0, 0, std::move(goal)};
        this->logger << "new goal set";
    }
}

void Typer::change_settings()
{
    std::string option_name, previous_mode;
    const uint16_t row_begin = 6, row_separate = 1;
    uint16_t current_row = row_begin;
    int16_t move         = 0;
    char key;

    clear_terminal();
    while(true) {
        terminal_jump_to(0, 0);
        std::cout << "\033[1;34mWelcome to TerminalTyper options menu!\n"
                  << "Use arrow 'UP'/'DOWN' to move around.\n"
                  << "Press 'ENTER' to choose\n"
                  << "You can press 'q' to go back.\n"
                  << RESET;
        for(int i = SETTINGS_FIRST; i <= SETTINGS_LAST; i++) {
            terminal_jump_to(row_begin + i * row_separate, 0);
            settings_item item = static_cast<settings_item>(i);
            option_name =
                current_row == row_begin + i * row_separate
                    ? OPTION_PICKED_COLOR + get_settings_name(item) + RESET
                    : get_settings_name(item);
            std::cout << option_name << std::endl;
        }
        terminal_jump_to(current_row, 2);
        std::cout.flush();
        key  = get_input();
        move = handle_up_down_arrow_key(key) * row_separate;
        if(key == ENTER) {
            switch((current_row - row_begin) / row_separate) {
                case MODE:
                    previous_mode = this->settings["mode"];
                    this->change_switch_option(
                        "mode",
                        {{"CLASSIC", CLASSIC_MODE}, {"TEXTS", TEXT_MODE}});
                    if(previous_mode != this->settings["mode"]) {
                        std::string path =
                            (this->settings["mode"] == CLASSIC_MODE ? "words"
                                                                    : "texts");
                        this->settings["words_filename"] =
                            path + "/" + get_first_file(path);
                        this->logger << "filename changed to: < " +
                                            this->settings["words_filename"] +
                                            " >";
                    }
                    break;
                case WORDS:
                    this->change_words_amount();
                    break;
                case FILENAME:
                    this->change_words_filename(
                        this->settings["mode"] == CLASSIC_MODE ? "words"
                                                               : "texts");
                    break;
                case TRAILING_CURSOR:
                    this->change_switch_option("trailing_cursor",
                                               {{"ON", "1"}, {"OFF", "0"}});
                    break;
                case SHOW_STATS:
                    this->change_switch_option("show_stats",
                                               {{"ON", "1"}, {"OFF", "0"}});
                    break;
                case RESTORE_DEFAULT:
                    this->load_default_settings();
                    break;
                case SAVE:
                    this->save_settings();
                case EXIT:
                    if(this->settings_changed) {
                        terminal_jump_to(
                            row_begin + (SETTINGS_LAST + 2) * row_separate, 0);
                        if(yes_no_question(
                               "You have unsaved changes to your "
                               "configuration, would u like to save?"))
                            this->save_settings();
                    }
                    return;
                default:
                    break;
            }
            clear_terminal();
        }
        else if(key == GO_BACK_SHORTCUT) {
            if(this->settings_changed) {
                terminal_jump_to(row_begin + (SETTINGS_LAST + 2) * row_separate,
                                 0);
                if(yes_no_question("You have unsaved changes to your "
                                   "configuration, would u like to save?"))
                    this->save_settings();
            }
            return;
        }
        else {
            switch_menu_item(move, current_row, row_begin,
                             row_begin + SETTINGS_LAST * row_separate);
        }
    }
}

void Typer::change_words_amount()
{
    std::stringstream ss;
    ss << DESCRIPTION_COLOR
       << "Change amount of words displayed for you to type\n"
       << "Note " << OPTION_CONFIG_COLOR << "this color" << RESET
       << DESCRIPTION_COLOR " means this setting is already chosen\n"
       << "Use arrow UP/DOWN to move around.\n"
       << "Press ENTER to choose words amount\n"
       << "Press 'q' to cancel\n"
       << RESET;
    std::string description = ss.str();

    std::string option_name, words_amount;
    const int16_t row_begin =
                      std::count(description.begin(), description.end(), '\n') +
                      2,
                  row_separate = 1;
    const uint16_t no_options = 10, value_jump = 5;
    const std::string element_before_option = "-> ",
                      words_setting_name    = "no_words";
    uint16_t current_row                    = row_begin;
    int16_t move                            = 0;
    char key;

    auto is_hovered = [&current_row, &row_begin](uint32_t option_number) {
        return (row_begin + option_number * row_separate) == current_row;
    };

    clear_terminal();
    while(true) {
        terminal_jump_to(0, 0);
        std::cout << description;
        for(int i = 0; i < no_options; i++) {
            terminal_jump_to(row_begin + i * row_separate, 0);
            words_amount = std::to_string((i + 1) * value_jump);
            if(this->is_from_config(words_amount, words_setting_name) &&
               !is_hovered(i))
                std::cout << OPTION_CONFIG_COLOR << element_before_option
                          << words_amount << RESET;
            else {
                option_name = is_hovered(i)
                                  ? OPTION_PICKED_COLOR +
                                        element_before_option + words_amount +
                                        RESET
                                  : element_before_option + words_amount;
                std::cout << option_name << std::endl;
            }
        }
        terminal_jump_to(current_row, 2);
        std::cout.flush();
        key  = get_input();
        move = handle_up_down_arrow_key(key) * row_separate;
        if(key == ENTER) {
            this->settings[words_setting_name] = std::to_string(
                value_jump * (((current_row - row_begin) / row_separate) + 1));
            this->settings_changed = true;
            this->logger << "words amount changed to: < " +
                                this->settings[words_setting_name] + " >";
            return;
        }
        else if(key == GO_BACK_SHORTCUT)
            return;
        else
            switch_menu_item(move, current_row, row_begin,
                             row_begin + (no_options - 1) * row_separate);
    }
}

void Typer::change_words_filename(const std::string &path)
{
    std::stringstream ss;
    ss << DESCRIPTION_COLOR << "Change words input filename (from " << path
       << "/ directory).\n"
       << "Note " << OPTION_CONFIG_COLOR << "this color" << RESET
       << DESCRIPTION_COLOR " means this setting is already chosen\n"
       << "Use arrow UP/DOWN to move around.\n"
       << "Press ENTER to choose file\n"
       << "Press 'q' to cancel\n"
       << RESET;
    std::string option_name, description = ss.str();

    const int16_t row_begin =
                      std::count(description.begin(), description.end(), '\n') +
                      2,
                  row_separate              = 1;
    const std::string element_before_option = "->> ",
                      filename_setting_name = "words_filename";
    uint16_t current_row                    = row_begin;
    int16_t move                            = 0;
    char key;
    auto files = this->get_filenames_vector(path);

    auto is_hovered = [&current_row, &row_begin](uint32_t option_number) {
        return (row_begin + option_number * row_separate) == current_row;
    };

    clear_terminal();
    while(true) {
        terminal_jump_to(0, 0);
        std::cout << description;
        for(uint32_t i = 0; i < files.size(); i++) {
            terminal_jump_to(row_begin + i * row_separate, 0);
            if(this->is_from_config(path + "/" + files.at(i),
                                    filename_setting_name) &&
               !is_hovered(i))
                std::cout << OPTION_CONFIG_COLOR << element_before_option
                          << files.at(i) << RESET;
            else {
                option_name = is_hovered(i)
                                  ? OPTION_PICKED_COLOR +
                                        element_before_option + files.at(i) +
                                        RESET
                                  : element_before_option + files.at(i);
                std::cout << option_name << std::endl;
            }
        }
        terminal_jump_to(current_row, 2);
        std::cout.flush();
        key  = get_input();
        move = handle_up_down_arrow_key(key) * row_separate;
        if(key == ENTER) {
            this->settings[filename_setting_name] =
                path + "/" +
                files.at(((current_row - row_begin) / row_separate));
            Generator::change_file(this->settings[filename_setting_name]);
            this->settings_changed = true;
            this->logger << "filename changed to: < " +
                                this->settings[filename_setting_name] + " >";
            return;
        }
        else if(key == GO_BACK_SHORTCUT)
            return;
        else
            switch_menu_item(move, current_row, row_begin,
                             row_begin + (files.size() - 1) * row_separate);
    }
}

void Typer::run()
{
    this->logger << "app run";
    this->select_menu();
}