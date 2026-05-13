#include <stddef.h>
#include <stdio.h>

#include "arguments.h"
#include "interpreter.h"

char* file_in = NULL;

int parse_arguments(int argc, char** argv) {
  struct arg_option options[] = {
    {
      .long_name = NULL,
      .short_name = 0,
      .type = arg_positional,
      .output = &file_in,
      .callback = NULL,
      .help = "Brainfuck source file"
    }
  };

  struct arg_parser parser = {
    .options = options,
    .option_count = sizeof(options) / sizeof(options[0])
  };

  int exit_code = arg_parse(&parser, argc, argv);
  if (exit_code != 0) {
    arg_print_help(&parser, argv[0]);
    return exit_code;
  }

  if (file_in == NULL) {
    fprintf(stderr, "No input file given!\n");
    arg_print_help(&parser, argv[0]);
    return 1;
  }

  return 0;
}

int main(int argc, char** argv) {
  if (parse_arguments(argc, argv) != 0) {
    return 1;
  }

  struct bf_state state;
  int result;

  result = bf_init_state(&state, 32);
  if (result != 0) {
    fprintf(stderr, "bf_init_state failed because: %s\n", state.error);
    bf_destroy_state(&state);
    return 1;
  }

  result = bf_load_code(&state, file_in);
  if (result != 0) {
    fprintf(stderr, "bf_load_code failed because: %s\n", state.error);
    bf_destroy_state(&state);
    return 1;
  }

  result = bf_run_code(&state, 0);
  /* we don't check for early exit, since we want to run all our code */
  if (result != 0) {
    fprintf(stderr, "bf_run_code failed because: %s\n", state.error);
    bf_dump_state(&state);
    bf_destroy_state(&state);
    return 1;
  }

  bf_dump_state(&state);

  bf_destroy_state(&state);

  return 0;
}
