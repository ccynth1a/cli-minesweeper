#ifndef TYPES_H
#define TYPES_H

#define X_MAX 15
#define Y_MAX 15
#define BUFFER_SIZE 256

#define MINE 'M'
#define EMPTY  '.'
#define FLAG 'F'

#define ANSI_RESET "\033[0m"

enum action_t {
  PLACE_FLAG = 'F', CHECK = 'C', UNKNOWN = 'U'
};

typedef struct {
  char display[Y_MAX][X_MAX]; 
  char true_layout[Y_MAX][X_MAX];
  unsigned int num_mines;
} game_t;

typedef struct {
  enum action_t action;
  unsigned int x;
  unsigned int y;
} input_t;

#endif // DEBUG
