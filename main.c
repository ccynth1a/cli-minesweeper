#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "types.h"
#include "main.h"
// global vars
game_t *pBoard;
bool visited[Y_MAX][X_MAX];

int main(int argc, char *argv[])
{
  srand(time(NULL)); // intialise random number generator
  pBoard = (game_t*)malloc(sizeof(game_t)); // declared on heap because why not
  initialise_screen_array(pBoard);
  generate_board(pBoard);

  char input[BUFFER_SIZE];
  // main game loop:
  // - draw screen
  // - hang on an input prompt
  // - parse that input prompt
  // - perform the appropriate action
  // - repeat
  while (1) {
    draw_screen(pBoard, false);
    printf("Type ! to open help menu\n");
    scanf("%s", input);
    input_t *parsed_input = parse_input(input);
    if (parsed_input == NULL) {
      continue;
    }
	switch (parsed_input->action) {
		case PLACE_FLAG:
			place_flag(pBoard, parsed_input->x, parsed_input->y);
			break;
		case CHECK:
			if (pBoard->true_layout[parsed_input->y][parsed_input->x] == MINE) {
				game_over(pBoard, parsed_input->x, parsed_input->y);
				free(parsed_input);
				exit(0);
			}
			check_squares(parsed_input->x, parsed_input->y);
	}
	free(parsed_input);
	if (flags_match_mines(pBoard)) {
		break;	
	}
  }
  printf("YOU WIN");
  return 0;
}

// Function that returns true only when the flags all match with the mines, and there are no empty squares
bool flags_match_mines(game_t *board)
{
	int num_correct_flags = 0;
	int num_flags = 0;
	int num_mines = board->num_mines;
	// TODO: this nesting is disgusting, please fix
	for (int row = 0; row < Y_MAX;row++) {
		for (int column = 0; column < X_MAX; column++) {
			if (board->display[row][column] == FLAG) {
				num_flags++;
				if (board->true_layout[row][column] == MINE) {
					num_correct_flags++;
				}
			} else if (board->display[row][column] == '#') {
				return false;
			}
		}
	}
	if (num_correct_flags == num_mines && num_flags == num_mines) {
		return true; // win condition
	} else {
		return false;
	}
}

// itoa() isnt in gcc stdlib so we ball
// NOTE: for some reason its in stdlib on windows but not linux????
char itoa(unsigned int value)  // only works for numbers 0-9. if it is detecting more than 9 mines im pretty sure something has gone horribly wrong
{
  return value + '0';
}

// Parse input for action, x and y and return an input_t struct
// C-X-Y expected input
input_t *parse_input(char *input)
{
  // parse C: char -> enum
  input_t *turn_input = malloc(sizeof(input_t));
  switch (toupper(input[0])) {
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
  turn_input->x = x - 'A'; // convert from ascii to integer. kinda funky but we ball
  // may have issues with the fact that 'a' is one, when the first index of the array is 0
  // parse Y: integer -> integer
  while (input[index] != '-') {
    index++;
  }
  int y = atoi(&input[index+1]);
  turn_input->y = y;
  return turn_input;
}

// recursively draw the screen with its borders
void draw_screen(game_t *board, bool true_layout)
{
  printf("\033[2J"); // ansi clear
  char (*screen)[X_MAX];
  if (true_layout) {screen = board->true_layout;} else {screen = board->display;}
  // print the top index row
  printf("\n    ");
  for (int i = 0; i < X_MAX; i++) {
    printf("%c ", i+65); // scuffed ascii math, should hold up just fine as long as boards dont exceed a 195
  }
  putc('\n', stdout);
	
  // print vertical bar
  for (int row = 0; row < Y_MAX; row++) {
    if (row >= 10) {
      printf("%d |", row);
    } else {
      printf("%d  |", row);
    }
    for (int column = 0; column < X_MAX; column++) {
	  char c = screen[row][column];	
	  // character codes
	  switch (c) {
		  case FLAG:
			  printf("\x1b[41m"); // red
			  break;
		  case '1':
			  printf("\033[0;32m"); //green
			  break;
		  case '2':
			  printf("\033[0;33m"); //yellow
			  break;
		  case '3':
			  printf("\033[0;34m"); //blue
			  break;
		  case '4':
			  printf("\033[0;35m"); //magenta
			  break;
		  case '5':
			  printf("\033[0;36m"); // cyan
			  break;
		  case '6':
			  printf("\x1b[38;5;225m"); // pink
			  break;
		  case '7':
			  printf("\x1b[38;5;220m"); // orange
			  break;
		  case '8':
			  printf("\x1b[38;5;129m"); // porpol
			  break;
	  }

      putc(screen[row][column], stdout);
	  printf(ANSI_RESET);
      putc(' ', stdout);
    }
    putc('\n', stdout);
  }
}

// toggle flag on a target position
void place_flag(game_t *board, int x, int y)
{
	if (board->display[y][x] == FLAG) {
		board->display[y][x] = '#';
		return;
	}
	board->display[y][x] = FLAG;
	printf("\033[2J");
	draw_screen(board, false);
}

void game_over(game_t *board, int x, int y) 
{
	board->display[y][x] = board->true_layout[y][x];
	printf("\033[2J");
	draw_screen(board, false);
	printf("GAME OVER");
}

// recursively check squares in a cardinal direction to produce the "cascade" effect when many empty squares border each other
int check_squares(int x, int y)
{
  // first check that input is in expected range.
  if (x < 0 || y < 0 || x > X_MAX || y > Y_MAX) {
    return -1; // return -1 if the user is being a dickhead
  }
  if (visited[y][x]) {
    return 0; // recursively return if the cell has already been visited
  }
  visited[y][x] = true;// if not visited, then mark it as so
  if (pBoard->true_layout[y][x] != MINE) {
	  pBoard->display[y][x] = pBoard->true_layout[y][x];// change the displayed character to be the real character
  }
  
  if (pBoard->true_layout[y][x] != EMPTY) { // reveal numbers (should not reveal bombs)
    return 0;
  }

  int directions[4][2] = { // all the cardinal directions mapped out for usage in iteration. Honestly no feckin clue which order its in
    {1,0}, {0,1}, {-1,0}, {0,-1}
  };
 
  for (int i = 0; i < 4; i++) {
    int new_x = x + directions[i][0];
    int new_y = y + directions[i][1];
    if (new_x >= 0 && new_x < X_MAX && new_y >= 0 && new_y < Y_MAX) { // segfault guarding
		check_squares(new_x, new_y);
    }
  }
}

// currently not bothered to put this in lmao
void help_menu()
{
  printf("MINESWEEPER HELP.\n"
  "Coordinates are written in format 'x-y' prefixed by action\n"

  "F - Place Flag on indicated square (e.g P-A-2)\n"
  "C - Check Square\n"
  "Q - Quit (req. Q-0-0)\n");
  sleep(3);
}

// fill entire array with '#'
void initialise_screen_array(game_t *board)
{
  for (int row = 0; row < Y_MAX; row++) {
    for (int column = 0;column < X_MAX; column++) {
      board->display[row][column] = '#';
    }
  }
}

// similar to check_squares() except 8 directional
unsigned int count_neighbours(int x, int y, game_t *board)
{
  unsigned int neighbours = 0;
  int directions[8][2] = {
    {1,0}, {1,1}, {1,-1},
    {0,1}, {0, -1}, {-1,0}, {-1, 1}, {-1,-1}
  };

  for (int i = 0; i < 8; i++) {
      int new_x = x + directions[i][0];
      int new_y = y + directions[i][1];
      if (new_x >= 0 && new_x < X_MAX && new_y >= 0 && new_y < Y_MAX) {
          if (board->true_layout[new_y][new_x] == MINE) {
			  neighbours++;
          }
      }
  }

  return neighbours;
}

// puts the numbers of the neighbouring squares on the board
int set_numbers(game_t *board)
{
  char buff[2]; // buffer for digit
  for (int row = 0; row < Y_MAX; row++) {
    for (int column = 0; column < X_MAX; column++) {
      if (board->true_layout[row][column] != MINE) {
        // have to guard to avoid segfaults here, but its nested pretty deep. excuse the extra function call
        unsigned int neighbours = count_neighbours(column, row, board);
		printf("%d ", neighbours);
		sprintf(buff, "%d", neighbours);
        if (neighbours == 0) {
			board->true_layout[row][column] = EMPTY;
		} else {
			board->true_layout[row][column] = buff[0];
		}
      }
    }
  }
}

// fill the board with randomised squares
void generate_board(game_t *board)
{
  int random_block;
  for (int row = 0; row < Y_MAX; row++) {
    for (int column = 0; column < X_MAX; column++) {
      random_block = rand() % 10;
	  if (random_block == 1) {
		  board->true_layout[row][column] = MINE;
		  board->num_mines++;
	  } else {
		  board->true_layout[row][column] = EMPTY;
	  }
    }
  }
  // now call the "set_numbers()" function to fill in the numbers
  set_numbers(board);
  draw_screen(board, true);
}






































