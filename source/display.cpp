#include "display.h"
#include <iostream>
#include <string>
#include <iterator>
#include <stdio.h>
#include <math.h>

void clear_top() {
    consoleSelect(&topScreen);
    //Cursor to top left
    std::cout << SCREEN_START_POINT;
    //Clear screen with empty lines
    for (int i = 0; i < CLEAR_SCREEN_LINES; i++)
        std::cout << std::string(70, ' ');
    std::cout << SCREEN_START_POINT;
}

void clear_bot() {
    consoleSelect(&bottomScreen);
    std::cout << SCREEN_START_POINT;
    for (int i = 0; i < CLEAR_SCREEN_LINES; i++)
        std::cout << std::string(60, ' ');
    std::cout << SCREEN_START_POINT;
}

void clear_save_status() {
    consoleSelect(&bottomScreen);
    printf(SAVE_STATUS_LINE);
    printf("                                               ");
    printf(SAVE_STATUS_LINE);
}

void print_version(std::string version) {
    consoleSelect(&bottomScreen);
    printf(VERSION_LINE);
    std::cout << version << std::endl;
}

void print_select_instructions(Config* cfg) {
    // so much code duplication...sad
    clear_bot();
    if (cfg->latest[0] != "") {
        std::string tmp_path(cfg->latest[0]);
        tmp_path += ";\n";
        printf("Press DPad Up to open\n");
        printf(tmp_path.c_str());
    }
    
    if (cfg->latest[1] != "") {
        std::string tmp_path(cfg->latest[1]);
        tmp_path += ";\n";
        printf("Press DPad Left to open\n");
        printf(tmp_path.c_str());
    }
    
    if (cfg->latest[2] != "") {
        std::string tmp_path(cfg->latest[2]);
        tmp_path += ";\n";
        printf("Press DPad Down to open\n");
        printf(tmp_path.c_str());
    }

    if (cfg->latest[3] != "") {
        std::string tmp_path(cfg->latest[3]);
        tmp_path += ";\n";
        printf("Press DPad Right to open\n");
        printf(tmp_path.c_str());
    }

    printf("Press SELECT to access main menu\n");
}

void print_instructions() {
    clear_bot();
    printf(INSTRUCTION_LINE);
	printf("Press A to select current line\n");
	printf("Press B to create a new file\n");
	printf("Press X to save file\n");
	printf("Press Y to open file\n");
    printf("Press R to search file\n");
    printf("Hold L to jump to top/end with up/down\n");
    printf("Press DPad or CPad to move up/down\n");
    printf("Press DPad Left to create a new line\n");
    printf("Press DPad Right to remove line\n");
    //printf("Press SELECT to access another menu\n");
	printf("Press START to exit\n");
}


std::string char_vec_to_string(std::vector<char>& line, Config* cfg) {

                std::string temp_str = "";
                int letters = 0;

                for (const auto& ch : line) {
                    if (letters < MAX_WIDTH) {
                        switch (ch) {
                            case '\t':
                            {
                                if (cfg->tab_spaces == 0) {
                                    temp_str.push_back(ch);
                                    letters++;
                                    break;
                                }
                                for (unsigned int i = 0; i < cfg->tab_spaces; i++) {
                                    temp_str.push_back(' ');
                                }
                                letters += cfg->tab_spaces;
                                break;
                            }
                            default:
                            {
                                //Store characters to display
                                temp_str.push_back(ch); 
                                letters++;
                                break;
                            }
                        }
                    } else {
                        //Too much text, truncate and display new line
                        temp_str.resize(MAX_WIDTH);
                        temp_str.push_back('\n');
                        break;
                    }
                }
                return temp_str;
}

std::string char_vec_to_string_counted(std::vector<char>& line, unsigned int curr_line, Config* cfg) {
    std::string temp_str(std::to_string(curr_line + 1) + '|');
    temp_str.append(char_vec_to_string(line, cfg));
    
    if (temp_str.length() > MAX_WIDTH)
    {
        temp_str.resize(MAX_WIDTH);
        temp_str.push_back('\n');
    }
    
    return temp_str;
}

void print_text(const char* str, unsigned int count, unsigned int selected_line) {

                if (count == selected_line)
                    if (str[0] == '\n') {
                        printf(SELECTED_TEXT_COLOUR);
                        printf("(empty line)");
                        printf(DEFAULT_TEXT_COLOUR);
                        printf("\n");
                    } else {
                        printf(SELECTED_TEXT_COLOUR);
                        printf("%s", str);
                        printf(DEFAULT_TEXT_COLOUR);
                    }
                else {
                    printf(DEFAULT_TEXT_COLOUR);
                    printf("%s", str);
                }
}

void print_save_status(std::string message) {
    clear_save_status();
    std::cout << message << std::endl;
}

void clear_line_status() {
    consoleSelect(&bottomScreen);
    printf(LINE_STATUS_LINE);
    printf("                                               ");
    printf(LINE_STATUS_LINE);
}

void print_line_status(unsigned int current_line) {
    clear_line_status();
    std::cout << "Current line: " << current_line+1;
}

void clear_directory_status() {
    consoleSelect(&bottomScreen);
    printf(DIRECTORY_LINE);
    printf("                                               ");
    printf(DIRECTORY_LINE);
}

void print_directory_status(std::string filename) {
    clear_directory_status();
    std::cout << "Current directory: " << filename;

}

void update_screen(File& file, unsigned int current_line, Config* cfg) {
    clear_top();
    consoleSelect(&bottomScreen);
    print_line_status(current_line);
    consoleSelect(&topScreen);
    unsigned int count = 0;
    
    std::string temp;

    //No scrolling needed
    if (file.lines.size() - 1 <= MAX_LINES) {
        for (auto iter = file.lines.begin(); iter != file.lines.end(); iter++) {
            //Print everything in the vector<char> that iterator points to
            
            if (cfg->show_line_number) {
                unsigned int line_cnt = std::distance(file.lines.begin(), iter);
                temp = char_vec_to_string_counted(*iter, line_cnt, cfg);
            } else {
                temp = char_vec_to_string(*iter, cfg);
            }

            const char* str_to_print = temp.c_str();
            print_text(str_to_print, count, current_line);
            count++;
        }
        
    //Scrolling needed
    } else {
    
        auto iter = file.lines.begin();
        
        if (current_line > 1 ) {
            advance(iter, (current_line -1));
        }
        else {
            advance(iter, current_line);
        }
        
        if (scroll == 0) {        
            for (int line = 0; line <= MAX_LINES; line++) {
                iter = file.lines.begin();
                advance(iter, line);
                
                if (cfg->show_line_number) {
                    unsigned int line_cnt = std::distance(file.lines.begin(), iter);
                    temp = char_vec_to_string_counted(*iter, line_cnt, cfg);
                } else {
                    temp = char_vec_to_string(*iter, cfg);
                }
                const char* str_to_print = temp.c_str();
                print_text(str_to_print, count, current_line);
                count++;
            }
            
        } else {
            for (int line = scroll; line <= MAX_LINES + scroll; line++) {
                iter = file.lines.begin();
                advance(iter, line);
                
                if (cfg->show_line_number) {
                    unsigned int line_cnt = std::distance(file.lines.begin(), iter);
                    temp = char_vec_to_string_counted(*iter, line_cnt, cfg);
                } else {
                    temp = char_vec_to_string(*iter, cfg);
                }
                const char* str_to_print = temp.c_str();
                print_text(str_to_print, count, current_line-scroll);
                count++;
                
            }
        }
    
    }

}

