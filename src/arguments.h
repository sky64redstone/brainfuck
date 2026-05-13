#ifndef ARGUMENTS_H
#define ARGUMENTS_H

enum arg_type {
  arg_flag,
  arg_int,
  arg_string,
  arg_positional
};

struct arg_option {
  /*
   * Long argument name will be used like: --myname [value]
   * arg_positional: serves as short description
   */
  const char* long_name;
  char short_name;
  enum arg_type type;
  /*
   * the type of the pointer depends on the type variable.
   * arg_flag:   int* (stores 1 or 0 depending if the flag was set or not)
   * arg_int:    int* (stores the int or keeps the original value)
   * arg_string: const char** (stores a pointer to the string at the pointer)
   * arg_positional: const char** (stores a pointer to the string at the pointer)
   */
  void* output;
  int (*callback)(const char*);
  const char* help;
};

struct arg_parser {
  struct arg_option* options;
  int option_count;
};

/*
 * Parses the arguments received from the main function and
 * compares it to the given arg_option's
 *
 * parser:  pointer to predefined options
 * argc:    number of received arguments
 * argv:    pointer to 'argc' different arguments
 *
 * returns: the exit code of the function (0: success, else failure)
 */
int arg_parse(struct arg_parser* parser, int argc, char** argv);

/*
 * Prints a string to stdout to show the usage of the given arg_option's.
 * Should be called, if arg_parse returns a non zero value.
 *
 * parser:  pointer to predefined options
 * name:    name of the executable (preferable the first parameter of argv
 */
void arg_print_help(struct arg_parser* parser, const char* name);
#endif

/*
 * USAGE:
 *
 * void(*mycallback)(const char* filename);
 * int verbosity = 0;
 *
 * struct arg_option options[] = {
 *   {
 *     .long_name = "verbose", // usage: --verbose
 *     .short_name = 'v', // usage: -v
 *     .type = arg_int,
 *     .output = &verbosity, // value will be stored here
 *     .callback = NULL, // will be called with the value
 *     .help = "Verbosity level. A value between 0 and 2 (default 0)"
 *   },
 *   {
 *     .long_name = "input file", // description
 *     .short_name = 0,
 *     .type = arg_positional, // first positional in options[]
 *                             // -> first positional argument
 *     .output = NULL,  // value will be stored here
 *     .callback = mycallback, // will be called with the value
 *     .help = "this will be ignored"
 *   }
 * };
 *
 * struct arg_parser parser = {
 *   .options = options,
 *   .option_count = sizeof(options)/sizeof(options[0])
 * };
 *
 * int exit_code = arg_parse(&parser, argc, argv);
 * if (exit_code != 0) {
 *   arg_print_help(&parser, argv[0]);
 *   return exit_code;
 * }
 *
 */
