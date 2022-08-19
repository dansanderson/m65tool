#include <config.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
  static int verbose_flag;
  // clang-format off
  static struct option long_options[] = {
    {"verbose", no_argument, &verbose_flag, 1},
    {"brief", no_argument, &verbose_flag, 0},
    {"create", required_argument, 0, 'c'},
    {0, 0, 0, 0}
  };
  // clang-format on

  int opt;
  int option_index;

  if (argc <= 1) {
    puts("no options or args");
  }

  while ((opt = getopt_long(argc, argv, "-ilw:", long_options,
                            &option_index)) != -1) {
    switch (opt) {
      case 0:
        // flag is an address of a flag var, or 0 for non-flags
        if (long_options[option_index].flag != 0) break;
        printf("option %s", long_options[option_index].name);
        if (optarg) printf(" with arg %s", optarg);
        printf("\n");
        break;

      case 1:
        printf("non-option arg amidst options: %s\n", optarg);
        break;

      case 'i':
        puts("option i");
        break;
      case 'l':
        puts("option l");
        break;
      case 'w':
        printf("option w = %s\n", optarg);
        break;
      case 'c':
        printf("long option create = %s\n", optarg);
        break;
      case '?':
        puts("(getopt_long printed an error message)");
        printf("opt=%d option_index=%d optopt=%d optarg=%s\n", opt,
               option_index, optopt, optarg);
        break;
      default:
        printf("opt = %d\n", opt);
        fprintf(stderr, "Usage: %s [-ilw] [file...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  printf("verbose_flag = %d\n", verbose_flag);
  for (int i = optind; i < argc; i++) {
    printf("arg at %d : %s\n", i, argv[i]);
  }
  return 0;
}
