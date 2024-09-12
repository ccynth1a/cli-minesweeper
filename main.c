#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "types.h"
// global vars
game_t board_struct;
bool visited[Y_MAX][X_MAX];


// debug function
void _show_true_layout(game_t *board)
{
  for (int row = 0; row < Y_MAX; row++) {
    for (int column = 0; column < X_MAX; column++) {
      putc(board->true_layout[row][column], stdout);
    }
  }
}

// itoa() isnt in gcc stdlib so we ball 

char itoa(unsigned int value)  // only works for numbers 0-9. if it is detecting more than 9 mines im pretty sure something has gone horribly wrong
{
  return value + '0';
}

// C-X-Y expected input
input_t *parse_input(char *input)
{
  // parse C: char -> enum
  input_t *turn_input = malloc(sizeof(input_t));
  switch (input[0]) {
    case PLACE_FLAG:
      turn_input->action = PLACE_FLAG;
      break;
    case CHECK:
      turn_input->action = CHECK;
      break;
    default:
      turn_input->action = UNKNOWN;
      break;
  }
  // parse X: char -> integer
  int index = 0;
  while (input[index] != '-') {
    index++;
  }
  char x = input[++index]; // bring index pointer to be to x
  // check its within expected range
  x = toupper(x);
  if (x > 'Z' || x < 'A') {
    return NULL;
  }
  turn_input->x = x - 'A' + 1; // convert from ascii to integer. kinda funky but we ball
  // may have issues with the fact that 'a' is one, when the first index of the array is 0
  // parse Y: integer -> integer
  while (input[index] != '-') {
    index++;
  }
  int y = atoi(&input[index]);
  turn_input->y = y;
  return turn_input;
}

int check_squares(int x, int y)
{
  // first check that input is in expected range.
  if (x < 0 || y < 0 || x > X_MAX || y > Y_MAX) {
    return -1; // return -1 if the user is being a dickhead
  }
  if (visited[y][x]) {
    return 0; // recursively return if the cell has already been visited
  }
  board_struct.display[y][x] = board_struct.true_layout[y][x];// change the displayed character to be an empty one, or the number of bombs nearby
  
  if (board_struct.true_layout[y][x] != EMPTY) {
    return 0;
  }

  int directions[4][2] = { // all the cardinal directions mapped out for usage in iteration. Honestly no feckin clue which order its in
    {1,0}, {0,1}, {-1,0}, {0,-1}
  };
 
  for (int i = 0; i < 8; i++) {
    int new_x = x + directions[i][0];
    int new_y = y + directions[i][1];
    if (new_x >= 0 && new_x < X_MAX && new_y >= 0 && new_y < Y_MAX) {
      if (board_struct.true_layout[new_y][new_x] == EMPTY) {
        check_squares(new_x, new_y);
      }
    }
  }
}

void help_menu()
{
  printf("MINESWEEPER HELP.\n"
  "Coordinates are written in format 'x-y' prefixed by action\n"

  "F - Place Flag on indicated square (e.g P-A-2)\n"
  "C - Check Square\n"
  "Q - Quit (req. Q-0-0)\n");
  sleep(3);
}

void initialise_screen_array(game_t *board)
{
  for (int row = 0; row < Y_MAX; row++) {
    for (int column = 0;column < X_MAX; column++) {
      board->display[row][column] = '#';
    }
  }
}

void draw_screen(game_t *board)
{
  // print the top index row
  printf("\033[2J");
  printf("   ");
  for (int i = 0; i < X_MAX; i++) {
    printf("%c ", i+65);
  }
  putc('\n', stdout);

  for (int row = 0; row < Y_MAX; row++) {
    if (row >= 10) {
      printf("%d ", row);
    } else {
      printf("%d  ", row);
    }
    for (int column = 0; column < X_MAX; column++) {
      putc(board->display[row][column], stdout);
      putc(' ', stdout);
    }
    putc('\n', stdout);
  }
}

unsigned int count_neighbours(int x, int y, game_t *board)
{
  unsigned int neighbours;
  int directions[8][2] = {
    {1,0}, {1,1}, {1,-1},
    {0,1}, {-1,0}, {0,-1}, {-1, 1}, {-1,-1}
  };

  for (int i = 0; i < 8; i++) {
      int new_x = x + directions[i][0];
      int new_y = y + directions[i][1];
      if (new_x >= 0 && new_x < X_MAX && new_y >= 0 && new_y < Y_MAX) {
          if (board_struct.true_layout[new_y][new_x] == EMPTY) {
              check_squares(new_x, new_y);
          }
      }
  }


/*
  // bottom middle
  if (y < Y_MAX) {
    if (board->true_layout[++y][x] == MINE) {++neighbours;}
  }
  
  // bottom left
  if (x > 0 && y < Y_MAX) {
    if (board->true_layout[++y][--x] == MINE) {++neighbours;}
  }

  // bottom right
  if (x < X_MAX && y < Y_MAX) {
    if (board->true_layout[++y][++x] == MINE) {++neighbours;}
  }

  // middle left
  if (x > 0) {
    if (board->true_layout[y][--x] == MINE) {++neighbours;}
  }

  // middle right
  if (x < X_MAX) {
    if (board->true_layout[y][++x] == MINE) {++neighbours;}
  }

  // top left
  if (x > 0 && y > 0) {
    if (board->true_layout[--y][--x] == MINE) {++neighbours;}
  }
  
  // top middle
  if (y > 0) {
    if (board->true_layout[--y][x] == MINE) {++neighbours;}
  } 

  // bottom right
  if (y > 0 && x < X_MAX) {
    if (board->true_layout[++x][--y] == MINE) {++neighbours;}
  }
*/
  return neighbours;
}

int set_numbers(game_t *board)
{
  for (int row = 0; row < Y_MAX; row++) {
    for (int column = 0; column < X_MAX; column++) {
      if (board->true_layout[row][column] != MINE) {
        // have to guard to avoid segfaults here, but its nested pretty deep. excuse the extra function call
        unsigned int neighbours = count_neighbours(column, row, board);
        if (neighbours == 0) {board->true_layout[row][column] = EMPTY;} else {board->true_layout[row][column] = itoa(neighbours);}
      }
    }
  }
}

void generate_board(game_t *board)
{
  int random_block;
  for (int row = 0; row < Y_MAX; row++) {
    for (int column = 0; column < X_MAX; column++) {
      random_block = rand() % 2;
      switch (random_block) {
        case 1:
          board->true_layout[row][column] = MINE;
          board->num_mines++;
        default:
          board->true_layout[row][column] = EMPTY;
      }
    }
  }
  // now call the "set_numbers()" function to fill in the numbers
  set_numbers(board);
}

int main(int argc, char *argv[])
{
  srand(time(NULL));
  initialise_screen_array(&board_struct);
  generate_board(&board_struct);

  char input[BUFFER_SIZE];
  // main game loop
  while (1) {
    draw_screen(&board_struct);
    printf("Type ! to open help menu\n");
    scanf("%s", input);
    switch (input[0]) {
      case '!':
        help_menu();
        continue;
      case '?':
        _show_true_layout(&board_struct);
        break;
      default:
        break;
    }
    input_t *parsed_input = parse_input(input);
    if (parsed_input == NULL) {
      exit(1);
    }
  }
  return 0;
}
