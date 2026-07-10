#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arguments.h"
#include "interpreter.h"

#define SHELL_DEFAULT_ERROR "No error"
#define SHELL_ERR_MALLOC "Failed to allocate enough memory"

#define SHELL_PROMPT "bf> "
#define SHELL_CONT_PROMPT "... "

char* file_in = NULL;
int verbose = 0;

void free_source(struct bf_state* state) {
  free(state->source);
  state->source = NULL;
  state->src_len = 0;
  state->src_ptr = 0;
}

int set_source_from_buffer(struct bf_state* state, const char* buffer, size_t length) {
  if (length == 0) {
    return 0;
  }

  state->source = malloc(length);
  if (!state->source) {
    state->error = SHELL_ERR_MALLOC;
    return 1;
  }

  memcpy(state->source, buffer, length);
  state->src_len = length;
  state->src_ptr = 0;
  state->error = SHELL_DEFAULT_ERROR;
  return 0;
}

int execute_buffer(struct bf_state* state, const char* buffer, size_t length) {
  int result;

  if (length == 0) {
    return 0;
  }

  result = set_source_from_buffer(state, buffer, length);
  if (result != 0) {
    return result;
  }

  result = bf_run_code(state, 0);
  free_source(state);
  return result;
}

int read_line(FILE* stream, char** out_line) {
  size_t capacity = 128;
  size_t length = 0;
  char* buffer = malloc(capacity);
  int ch;

  if (!buffer) {
    return -1;
  }

  while ((ch = fgetc(stream)) != EOF) {
    if (ch == '\n') {
      break;
    }

    if (length + 1 >= capacity) {
      size_t new_capacity = capacity * 2;
      char* resized = realloc(buffer, new_capacity);

      if (!resized) {
        free(buffer);
        return -1;
      }

      buffer = resized;
      capacity = new_capacity;
    }

    buffer[length++] = (char)ch;
  }

  if (ch == EOF && length == 0) {
    free(buffer);
    return 0;
  }

  buffer[length] = '\0';
  *out_line = buffer;
  return 1;
}

void trim_trailing_cr(char* text) {
  size_t length;

  if (!text) {
    return;
  }

  length = strlen(text);
  if (length > 0 && text[length - 1] == '\r') {
    text[length - 1] = '\0';
  }
}

int bracket_balance_delta(const char* text) {
  int balance = 0;

  for (const char* p = text; *p != '\0'; ++p) {
    if (*p == '[') {
      balance++;
    } else if (*p == ']') {
      balance--;
    }
  }

  return balance;
}

int append_text(
  char** buffer,
  size_t* length,
  size_t* capacity,
  const char* text
) {
  size_t addition = strlen(text);
  size_t required = *length + addition + 1;
  char* resized;

  if (required > *capacity) {
    size_t new_capacity = (*capacity == 0) ? 64 : *capacity;

    while (new_capacity < required) {
      new_capacity *= 2;
    }

    resized = realloc(*buffer, new_capacity);
    if (!resized) {
      return 1;
    }

    *buffer = resized;
    *capacity = new_capacity;
  }

  memcpy(*buffer + *length, text, addition);
  *length += addition;
  (*buffer)[*length] = '\0';
  return 0;
}

void print_shell_help(void) {
  puts("Brainfuck shell commands:");
  puts("  :help   show this help");
  puts("  :dump   dump interpreter state");
  puts("  :reset  clear tape and move pointer to cell 0");
  puts("  :quit   exit the shell");
}

void reset_tape(struct bf_state* state) {
  if (state->cells && state->cells_count > 0) {
    memset(state->cells, 0, state->cells_count * sizeof(char));
  }
  state->cell_ptr = 0;
  state->src_ptr = 0;
  state->error = SHELL_DEFAULT_ERROR;
}

int handle_shell_command(struct bf_state* state, const char* line) {
  if (strcmp(line, ":help") == 0 || strcmp(line, ":h") == 0) {
    print_shell_help();
    return 0;
  }

  if (strcmp(line, ":dump") == 0 || strcmp(line, ":d") == 0) {
    bf_dump_state(state);
    return 0;
  }

  if (strcmp(line, ":reset") == 0 || strcmp(line, ":r") == 0) {
    reset_tape(state);
    return 0;
  }

  if (strcmp(line, ":quit") == 0 || strcmp(line, ":q") == 0 || strcmp(line, ":exit") == 0) {
    return 1;
  }

  return -1;
}

int run_shell(struct bf_state* state) {
  char* line = NULL;
  char* program = NULL;
  size_t program_length = 0;
  size_t program_capacity = 0;
  int bracket_balance = 0;

  puts("Brainfuck shell. Type :help for commands.");

  for (;;) {
    int read_result;
    int command_result;
    int result;

    fputs(program_length == 0 ? SHELL_PROMPT : SHELL_CONT_PROMPT, stdout);
    fflush(stdout);

    read_result = read_line(stdin, &line);
    if (read_result < 0) {
      fprintf(stderr, "Failed to read input.\n");
      free(line);
      free(program);
      return 1;
    }

    if (read_result == 0) {
      free(line);
      break;
    }

    trim_trailing_cr(line);

    if (program_length == 0 && line[0] == ':') {
      command_result = handle_shell_command(state, line);
      free(line);
      line = NULL;

      if (command_result > 0) {
        break;
      }
      if (command_result == 0) {
        continue;
      }
    }

    if (program_length > 0) {
      if (append_text(&program, &program_length, &program_capacity, "\n") != 0) {
        fprintf(stderr, "Failed to allocate enough memory.\n");
        free(line);
        free(program);
        return 1;
      }
    }

    if (append_text(&program, &program_length, &program_capacity, line) != 0) {
      fprintf(stderr, "Failed to allocate enough memory.\n");
      free(line);
      free(program);
      return 1;
    }

    bracket_balance += bracket_balance_delta(line);

    free(line);
    line = NULL;

    if (bracket_balance > 0) {
      continue;
    }

    if (program_length == 0) {
      continue;
    }

    result = execute_buffer(state, program, program_length);
    if (result != 0) {
      fprintf(stderr, "bf_run_code failed because: %s\n", state->error);
      if (verbose) {
        bf_dump_state(state);
      }
    }

    free(program);
    program = NULL;
    program_length = 0;
    program_capacity = 0;
    bracket_balance = 0;
  }

  free(program);
  return 0;
}

int parse_arguments(int argc, char** argv) {
  struct arg_option options[] = {
    {
      .long_name = "source",
      .short_name = 0,
      .type = arg_positional,
      .output = &file_in,
      .callback = NULL,
      .help = "Brainfuck source file"
    },
    {
      .long_name = "verbose",
      .short_name = 'v',
      .type = arg_flag,
      .output = &verbose,
      .callback = NULL,
      .help = "Enable dumping debug information to stderr"
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

  return 0;
}

int main(int argc, char** argv) {
  struct bf_state state = {0};
  int result;

  if (parse_arguments(argc, argv) != 0) {
    return 1;
  }

  result = bf_init_state(&state, 32);
  if (result != 0) {
    fprintf(stderr, "bf_init_state failed because: %s\n", state.error);
    if (verbose) {
      bf_dump_state(&state);
    }
    return 1;
  }

  if (file_in != NULL) {
    result = bf_load_code(&state, file_in);
    if (result != 0) {
      fprintf(stderr, "bf_load_code failed because: %s\n", state.error);
      if (verbose) {
        bf_dump_state(&state);
      }
      bf_destroy_state(&state);
      return 1;
    }

    result = bf_run_code(&state, 0);
    if (result != 0) {
      fprintf(stderr, "bf_run_code failed because: %s\n", state.error);
      if (verbose) {
        bf_dump_state(&state);
      }
      bf_destroy_state(&state);
      return 1;
    }
  } else {
    result = run_shell(&state);
    if (result != 0) {
      if (verbose) {
        bf_dump_state(&state);
      }
      bf_destroy_state(&state);
      return 1;
    }
  }

  if (verbose) {
    bf_dump_state(&state);
  }

  bf_destroy_state(&state);
  return 0;
}
