#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include "file.h"
#include "display.h"
#include "file_io.h"
#include "config.h"

#define BUFFER_SIZE 1025    //Notepad's line limit + \0
#define MAX_BOTTOM_SIZE 28

#define VERSION "Notepad3DS Version 1.1.5"

PrintConsole topScreen, bottomScreen;
int scroll = 0;
bool fast_scroll = false;
bool select_menu = false;

void move_down(File file, Config* cfg);
void move_up(File file, Config* cfg);
void write_file(File file);
void main_open_file(Config* cfg, std::string filename, File file);

unsigned int curr_line = 0;

Config cfg;

int main(int argc, char **argv)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, &topScreen);
    consoleInit(GFX_BOTTOM, &bottomScreen);
    consoleSelect(&bottomScreen);

    std::string err_msg;

    load_config(&cfg, &err_msg);

    // Show error if something happened
    if (err_msg != "")
    {
        consoleSelect(&bottomScreen);
        printf(SCREEN_START_POINT);
        std::cout << err_msg << std::endl << "Press any key to exit." << std::endl;
        while (aptMainLoop())
        {
            hidScanInput();

		    u32 kDown = hidKeysDown();

            if (kDown)
                break;
        }
        gfxExit();
	    return 0;
    }

    //Software keyboard thanks to fincs
    print_instructions();

    print_version(VERSION);
    
    File file;      //Use as default file

    update_screen(file, curr_line, &cfg);

	while (aptMainLoop())
	{

		hidScanInput();

		u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

		if (kDown & KEY_START)
			break;

		static SwkbdState swkbd;
		static char mybuf[BUFFER_SIZE];
		SwkbdButton button = SWKBD_BUTTON_NONE;
		bool didit = false;

        swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, -1);
        swkbdSetValidation(&swkbd, SWKBD_ANYTHING, SWKBD_ANYTHING, 2);
        swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN);
        if (!select_menu) {
            if (kDown & KEY_A) {
                //Select current line for editing
                swkbdSetHintText(&swkbd, "Input text here.");
                //Iterator to find current selected line
                auto line = file.lines.begin();
                if (curr_line < file.lines.size())
                {
                    if (curr_line != 0)
                        advance(line, curr_line);
                    
                    if (curr_line == file.lines.size() - 1) {
                        file.lines.push_back(std::vector<char>{'\n'});
                    }
                    //Need a char array to output to keyboard
                    char current_text[BUFFER_SIZE] = "";
                    copy(line->begin(), line->end(), current_text);
                    swkbdSetInitialText(&swkbd, current_text);
                }
                didit = true;
                button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
            }

            if (kDown & KEY_DLEFT) {
                if (curr_line < file.lines.size()) {
                    auto line = file.lines.begin();
                    advance(line, curr_line + 1);
                    file.lines.insert(line, std::vector<char>{'\n'});
                    curr_line++;
                    update_screen(file, curr_line, &cfg);
                }
            }

            if (kDown & KEY_DRIGHT) {
                if (curr_line < file.lines.size()) {
                    auto line = file.lines.begin();
                    if (curr_line > 0)
                        advance(line, curr_line);
                    file.lines.erase(line);
                    update_screen(file, curr_line, &cfg);
                }
            }

            if (kDown & KEY_B) {
                //Create new file
                
                //Clear buffer
                memset(mybuf, '\0', BUFFER_SIZE);
                
                swkbdSetHintText(&swkbd, "Input filename here."); 
                button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
                std::string filename = "";
                for (int i = 0; mybuf[i] != '\0'; i++)
                    filename.push_back(mybuf[i]);

                if (filename == "")
                    print_save_status("No new file created");
                else if (std::filesystem::exists(std::filesystem::path(filename)))
                    print_save_status("File on this path already exists!");
                else {
                    File new_file;
                    file = new_file;
                    file.filename = filename;
                    curr_line = 0;
                    scroll = 0;
                    write_file(file);
                    update_screen(file, curr_line, &cfg);
                }
            }

            if (kDown & KEY_R) {
                //find a thing
                
                //Clear buffer
                memset(mybuf, '\0', BUFFER_SIZE);
                //Get term to search for
                swkbdSetHintText(&swkbd, "Input search term here."); 
                button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
                int line = file.find(mybuf);
                if (line < 0)
                    printf("Could not find %s", mybuf);
                else {
                    printf("Found %s at %d", mybuf, line);
                    curr_line = line;
                    if (curr_line > MAX_BOTTOM_SIZE) {
                        scroll = curr_line - MAX_BOTTOM_SIZE;
                    }
                    update_screen(file, curr_line, &cfg);
                }   

            }

            fast_scroll = (kHeld & KEY_L);

            if (kDown & KEY_X) {
                write_file(file);
            }

            if (kDown & KEY_Y) {
                memset(mybuf, '\0', BUFFER_SIZE);

                //Get file name
            
                swkbdSetHintText(&swkbd, "Input filename here."); 
                button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
                std::string filename = "";
                for (int i = 0; mybuf[i] != '\0'; i++)
                    filename.push_back(mybuf[i]);

                main_open_file(&cfg, filename, file);
            }

            if (kDown & KEY_DDOWN) {
                //Move a line down (towards bottom of screen)
                move_down(file, &cfg); 
            }

            if (kHeld & KEY_CPAD_DOWN) {
                //Move a line down (towards bottom of screen)
                //as long as down is held
                move_down(file, &cfg); 
            }
            if (kDown & KEY_DUP) {
                //Move a line up (towards top of screen)
                move_up(file, &cfg);
            }


            if (kHeld & KEY_CPAD_UP) {
                //Move a line up (towards top of screen)
                //as long as up is held
                move_up(file, &cfg);
            }

            if (didit)
            {
                if (button != SWKBD_BUTTON_NONE)
                {
                    std::vector<char> new_text = char_arr_to_vector(mybuf);

                    if (curr_line >= file.lines.size()) {
                        //Empty line, add a new one.
                        file.add_line(new_text);
                    } else {
                        file.edit_line(new_text, curr_line);
                    }
                    update_screen(file, curr_line, &cfg);
                } else
                    printf("swkbd event: %d\n", swkbdGetResult(&swkbd));
            }
        } else {
            if (kDown & KEY_DUP) {
                main_open_file(&cfg, cfg.latest[0].string(), file);
            }
            if (kDown & KEY_DLEFT) {
                main_open_file(&cfg, cfg.latest[1].string(), file);
            }
            if (kDown & KEY_DDOWN) {
                main_open_file(&cfg, cfg.latest[2].string(), file);
            }
            if (kDown & KEY_DRIGHT) {
                main_open_file(&cfg, cfg.latest[3].string(), file);
            }
        }
        if (kDown & KEY_SELECT) {
            select_menu = !select_menu;
            if (select_menu) {
                print_select_instructions(&cfg);
            } else {
                print_instructions();
            }
        }

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		gspWaitForVBlank();
	}

	gfxExit();
	return 0;
}

void move_down(File file, Config* cfg) {
    //Move a line down (towards bottom of screen)
    if (curr_line < file.lines.size() - 1) {
        if (fast_scroll) {
            curr_line = file.lines.size()-1;
            scroll = curr_line - MAX_BOTTOM_SIZE;

        } else {

            if ( (curr_line - scroll >= MAX_BOTTOM_SIZE) && (curr_line < file.lines.size() ) ) {
                scroll++;
                curr_line++;
            } else {
                curr_line++;
            }
        }
        update_screen(file, curr_line, cfg);
    }

}

void move_up(File file, Config* cfg) {
    //Move a line up (towards top of screen)

    if (curr_line != 0) {
        if (fast_scroll) {
            //Jump to the top
            curr_line = 0;
            scroll = 0;

        } else {

            curr_line--;
            if (curr_line - scroll <= 0 && scroll != 0) {
                scroll--;
            }
        }
        update_screen(file, curr_line, cfg);
    }
}

void write_file(File file) {
    if (file.filename == "") {
        clear_save_status();
        std::cout << "Can't save empty filename" << std::endl;
        consoleSelect(&topScreen);
        return;
    }

    bool success = write_to_file(file.filename, file);
    
    if (success) {
        print_save_status("File written to " + file.filename);
        print_directory_status(file.filename);
    } else {
        print_save_status("Failed to write " + file.filename);
    }
}

void main_open_file(Config* cfg, std::string filename, File file) {
    if (filename == "") {
        clear_save_status();
        std::cout << "Can't open empty file" << std::endl;
        consoleSelect(&topScreen);
        return;
    }
    
    curr_line = 0;
    scroll = 0;

    File oldfile = file;
    file = open_file(filename);
    
    if (file.read_success) {
        update_screen(file, curr_line, cfg);
        clear_save_status();
        std::cout << "Successfully opened " << filename << std::endl;
        clear_directory_status();
        std::cout << "Current file: " << filename;
        consoleSelect(&topScreen);
        update_latest(cfg, filename);
    } else {
        file = oldfile;
        update_screen(file, curr_line, cfg);
        clear_save_status();
        std::cout << "Failed to open " << filename << std::endl;
        consoleSelect(&topScreen);
    }
}