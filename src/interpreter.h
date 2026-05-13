#ifndef INTERPRETER_H
  #define INTERPRETER_H

  #include <stddef.h>

  struct bf_state {
    /* Code */
    char* source;
    size_t src_len;
    /* Cells */
    char* cells;
    size_t cells_count;
    /* Pointers */
    size_t cell_ptr;
    size_t src_ptr;
    /* Error message */
    const char* error;
  };

  /*
   * Initializes the members of the bf_state struct.
   *
   * state:         pointer to bf_state object
   * initial_cells: number of cells to allocate (0 for default)
   *
   * returns: exit code (0: success, else failure)
   */
  int bf_init_state(struct bf_state* state, int initial_cells);

  /*
   * Loads the Brainfuck code from a file.
   *
   * state:    pointer to initialized bf_state object
   * filename: name of the Brainfuck file
   *
   * returns: exit code (0: success, else failure)
   */
  int bf_load_code(struct bf_state* state, const char* filename);

  /*
   * Runs the Brainfuck code from memory.
   *
   * state:        pointer to initialized bf_state object with code
   * instructions: number of instructions to execute before returning
   *               (0: run all)
   *
   * returns: 0: code exited, 1: success and exiting early, else failure
   */
  int bf_run_code(struct bf_state* state, size_t instructions);

  /*
   * Dumps the current bf_state readable to stderr.
   *
   * state: Brainfuck state to dump
   */
  void bf_dump_state(struct bf_state* state);

  /*
   * Releases all resources used by given bf_state.
   *
   * state: Brainfuck state
   */
  void bf_destroy_state(struct bf_state* state);

#endif
