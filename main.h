#ifndef MAIN_H
#define MAIN_H

#include "types.h"
#include <stdbool.h>

bool flags_match_mines(game_t *board);

char itoa(unsigned int value);

input_t *parse_input(char *input);

void draw_screen(game_t *board, bool true_layout);

void place_flag(game_t *board, int x, int y);

void game_over(game_t *board, int x, int y);

int check_squares(int x, int y);

void help_menu();

void initialise_screen_array(game_t *board);

unsigned int count_neighbours(int x, int y, game_t *board);

int set_numbers(game_t *board);

void generate_board(game_t *board);

#endif // !MAIN_H
