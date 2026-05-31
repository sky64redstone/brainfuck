#include "arguments.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void arg_print_help(struct arg_parser* parser, const char* name) {
  if (name == NULL) {
    name = "program-name";
  }

  if (parser == NULL || parser->options == NULL) {
    fprintf(
      stderr,
      "Given parameter 'parser' is a nullpointer!\n"
      "File: %s:%i %s()\n", __FILE__, __LINE__, __func__
    );
    return;
  }

  printf("Usage: %s [options]", name);

  for (int i = 0; i < parser->option_count; ++i) {
    struct arg_option* o = &parser->options[i];

    if (o->type == arg_positional) {
      printf(" <%s>", o->long_name ? o->long_name : "arg");
    }
  }

  printf("\n\n");

  for (int i = 0; i < parser->option_count; ++i) {
    struct arg_option* o = &parser->options[i];

    if (o->type != arg_positional) {
      printf("    -%c, --%-12s %s\n", o->short_name, o->long_name, o->help);
    }
  }
}

struct arg_option* find_option(struct arg_parser* parser, const char* long_name, char short_name, int* start_index) {
  if (parser == NULL || parser->options == NULL) {
    fprintf(
      stderr,
      "Given parameter 'parser' is a nullpointer!\n"
      "File: %s:%i %s()\n", __FILE__, __LINE__, __func__
    );
    return NULL;
  }

  for (int i = start_index ? *start_index : 0; i < parser->option_count; ++i) {
    struct arg_option* o = &parser->options[i];
    if (
      (long_name && o->long_name && strcmp(long_name, o->long_name) == 0) ||
      (short_name && short_name == o->short_name) ||
      (long_name == NULL && short_name == (char)0 && o->type == arg_positional)
    ) {
      if (start_index) {
        *start_index = i + 1;
      }
      return o;
    }
  }

  return NULL;
}

/*
 * TODO:
 *
 * - allow stacking flags (instead "-a -b" do "-ab")
 * - allow no spaces with ints (allow "-v2" -> "-v 2")
 */
int arg_parse(struct arg_parser* parser, int argc, char** argv) {
  if (parser == NULL || parser->options == NULL) {
    fprintf(
      stderr,
      "Given parameter 'parser' is a nullpointer!\n"
      "File: %s:%i %s()\n", __FILE__, __LINE__, __func__
    );
    return 1;
  }
  if (argv == NULL) {
    fprintf(
      stderr,
      "Given parameter 'argv' is a nullpointer!\n"
      "File: %s:%i %s()\n", __FILE__, __LINE__, __func__
    );
    return 1;
  }

  int positional_index = 0;

  for (int i = 1; i < argc; ++i) {
    char* arg = argv[i];

    if (arg == NULL) {
      continue;
    }

    if (strncmp(arg, "--", 2) == 0) {
      struct arg_option* opt = find_option(parser, arg + 2, 0, NULL);
      if (!opt) {
        return -1;
      }

      const char* value = NULL;
      if (opt->type != arg_flag) {
        if (++i >= argc) {
          return -2;
        }
        value = argv[i];
      }

      if (opt->output) {
        switch (opt->type) {
          case arg_flag:
            *(int*)opt->output = 1;
            break;
          case arg_int:
            *(int*)opt->output = atoi(value);
            break;
          case arg_string:
            *(const char**)opt->output = value;
            break;
          default:
            break;
        }
      }

      if (opt->callback) {
        opt->callback(value);
      }
    }

    else if (arg[0] == '-' && arg[1] != '\0') {
      struct arg_option* opt = find_option(parser, NULL, arg[1], NULL);
      if (!opt) {
        return -3;
      }

      const char* value = NULL;
      if (opt->type != arg_flag) {
        if (++i >= argc) {
          return -4;
        }
        value = argv[i];
      }

      if (opt->output) {
        switch (opt->type) {
          case arg_flag:
            *(int*)opt->output = 1;
            break;
          case arg_int:
            *(int*)opt->output = atoi(value);
            break;
          case arg_string:
            *(const char**)opt->output = value;
            break;
          default:
            break;
        }
      }

      if (opt->callback) {
        opt->callback(value);
      }
    }

    else {
      struct arg_option* opt = find_option(parser, NULL, 0, &positional_index);
      if (!opt) {
        return -5;
      }

      if (opt->output) {
        *(const char**)opt->output = arg;
      }

      if (opt->callback) {
        opt->callback(arg);
      }
    }
  }
  return 0;
}
