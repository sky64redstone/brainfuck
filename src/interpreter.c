#include "interpreter.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> /* SIZE_MAX */
#include <string.h>

#define INSTR_INVALID (char)(0xFF)
#define INSTR_END '\0'
#define VALID_CELL 0b01
#define VALID_SRC  0b10

#define ERR_NO "No error"
#define ERR_ALREADY_INIT "State is already initialized"
#define ERR_ALREADY_CODE "State contains already code"
#define ERR_MALLOC "Failed to allocate enough memory"
#define ERR_OPEN_FILE "Failed to open the specified file"
#define ERR_FILE_SIZE "Failed to determine file size"
#define ERR_NO_CELLS "State has no initialized cells"
#define ERR_NO_CODE "State contains no code"
#define ERR_PTR_ZERO "Cell pointer moved below zero"
#define ERR_MATCHING_BRACKET "Coudn't find matching bracket"

int bf_valid_state(struct bf_state* state) {
  int result = 0;
  result |= state->source != NULL && state->src_len != 0 ? VALID_SRC : 0;
  result |= state->cells != NULL && state->cells_count != 0 ? VALID_CELL : 0;
  return result;
}

int bf_init_state(struct bf_state* state, int initial_cells) {
  if (bf_valid_state(state) & VALID_CELL) {
    state->error = ERR_ALREADY_INIT;
    return 1;
  }
  state->source = NULL;
  state->src_len = 0;
  state->cells = calloc(initial_cells, sizeof(char));
  state->cells_count = state->cells ? initial_cells : 0;
  state->cell_ptr = 0;
  state->src_ptr = 0;
  state->error = state->cells ? ERR_NO : ERR_MALLOC;
  return !state->cells;
}

int bf_load_code(struct bf_state* state, const char* filename) {
  if (bf_valid_state(state) & VALID_SRC) {
    state->error = ERR_ALREADY_CODE;
    return 1;
  }

  FILE* file = fopen(filename, "rb");
  if (!file) {
    state->error = ERR_OPEN_FILE;
    return 1;
  }

  /* read length */
  fseek(file, 0L, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  if (file_size < 0) {
    state->error = ERR_FILE_SIZE;
    fclose(file);
    return 1;
  }

  state->src_len = (size_t)file_size;

  /* read data */
  state->source = malloc(state->src_len);
  if (!state->source) {
    state->error = ERR_MALLOC;
    fclose(file);
    return 1;
  }

  size_t read_size = fread(state->source, 1, state->src_len, file);
  state->src_len = read_size;

  fclose(file);

  return 0;
}

int bf_expand_cells(struct bf_state* state) {
  if ((!bf_valid_state(state) & VALID_CELL)) {
    state->error = ERR_NO_CELLS;
    return 1;
  }

  size_t new_size = state->cells_count << 1;
  char* new_cells = realloc(state->cells, new_size);

  if (new_cells == NULL) {
    state->error = ERR_MALLOC;
    return 1;
  }

  memset(new_cells + state->cells_count, 0, state->cells_count);

  state->cells_count = new_size;
  state->cells = new_cells;

  return 0;
}

char bf_next_instr(struct bf_state* state) {
  if (!(bf_valid_state(state) & VALID_SRC)) {
    state->error = ERR_NO_CODE;
    return INSTR_INVALID;
  }

  if (state->src_ptr >= state->src_len) {
    return INSTR_END;
  }
  return state->source[state->src_ptr++];
}

size_t bf_next_bracket(struct bf_state* state) {
  if (!(bf_valid_state(state) & VALID_SRC)) {
    state->error = ERR_NO_CODE;
    return SIZE_MAX;
  }

  size_t depth = 0;

  for (size_t i = state->src_ptr; i < state->src_len; i++) {
    if (state->source[i] == '[') {
      depth++;
    }
    else if (state->source[i] == ']') {
      if (depth == 0) {
        return i;
      }
      depth--;
    }
  }

  return SIZE_MAX;
}

size_t bf_prev_bracket(struct bf_state* state) {
  if (!(bf_valid_state(state) & VALID_SRC)) {
    state->error = ERR_NO_CODE;
    return 1;
  }

  size_t depth = 0;

  if (state->src_ptr < 2) {
    return SIZE_MAX;
  }

  for (size_t i = state->src_ptr - 2; i > 0; i--) {
    if (state->source[i] == ']') {
      depth++;
    }
    else if (state->source[i] == '[') {
      if (depth == 0) {
        return i;
      }
      depth--;
    }
  }

  if (state->source[0] == '[' && depth == 0) {
    return 0;
  }

  return SIZE_MAX;
}

int bf_run_code(struct bf_state* state, size_t instructions) {
  if (instructions == 0) {
    instructions = SIZE_MAX;
  }

  for (size_t i = 0; i < instructions; i++) {
    char c = bf_next_instr(state);

    if (c == INSTR_END) {
      fflush(stdout);
      return 0; /* Code exited */
    }
    if (c == INSTR_INVALID) {
      fflush(stdout);
      return 2;
    }

    switch (c) {
      case '>': {
        state->cell_ptr++;

        if (state->cell_ptr >= state->cells_count) {
          int result = bf_expand_cells(state);
          if (result != 0) {
            return 2;
          }
        }

        break;
      }
      case '<': {
        if (state->cell_ptr == 0) {
          state->error = ERR_PTR_ZERO;
          return 2;
        }
        state->cell_ptr--;
        break;
      }

      case '+': state->cells[state->cell_ptr]++; break;
      case '-': state->cells[state->cell_ptr]--; break;

      case '.': putchar(state->cells[state->cell_ptr]); break;
      case ',': {
        fflush(stdout); /* Input may depend on previous output */
        int input = getchar();
        state->cells[state->cell_ptr]= (input == EOF) ? 0 : (char)input;
        break;
      }

      case '[': {
        /* skip loop if the current cell is 0 */
        if (state->cells[state->cell_ptr] == 0) {
          size_t next = bf_next_bracket(state);
          if (next == SIZE_MAX) {
            state->error = ERR_MATCHING_BRACKET;
            return 2;
          }
          state->src_ptr = next + 1;
        }
        break;
      }
      case ']': {
        /* only loop if the current cell is not 0 */
        if (state->cells[state->cell_ptr] != 0) {
          size_t prev = bf_prev_bracket(state);
          if (prev == SIZE_MAX) {
            state->error = ERR_MATCHING_BRACKET;
            return 2;
          }
          state->src_ptr = prev + 1;
        }
        break;
      }

      default: break;
    }

  } // for

  fflush(stdout);

  return 1; /* we reached the limit of instructions we needed to execute */
}

void bf_dump_state(struct bf_state* state) {
  fprintf(stderr, "\n=== dump ===\n");

  fprintf(stderr, "Program:\n");
  fwrite(state->source, 1, state->src_len, stderr);
  fprintf(stderr, "\n");

  fprintf(stderr, "src_ptr: %zu / %zu\n", state->src_ptr, state->src_len);

  if (state->src_ptr < state->src_len) {
    fprintf(stderr, "current instruction: '%c'\n", state->source[state->src_ptr]);
  } else {
    fprintf(stderr, "current instruction: <EOF>\n");
  }

  fprintf(stderr, "cell ptr: %d\n", state->cell_ptr);
  fprintf(stderr, "cells:\n");

  for (size_t i = 0; i < state->cells_count; i++) {
    if ((int)i == state->cell_ptr) {
      fprintf(stderr, "[%3u]", state->cells[i]);
    } else {
      fprintf(stderr, " %3u ", state->cells[i]);
    }

    if ((i + 1) % 16 == 0) {
      fprintf(stderr, "\n");
    }
  }

  fprintf(stderr, "\n============\n");
}

void bf_destroy_state(struct bf_state* state) {
  free(state->source);
  free(state->cells);
  state->source = NULL;
  state->src_len = 0;
  state->cells = NULL;
  state->cells_count = 0;
  state->cell_ptr = 0;
  state->src_ptr = 0;
  state->error = ERR_NO;
}
