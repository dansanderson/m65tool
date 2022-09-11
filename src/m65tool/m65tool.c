#include <config.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "datastruct/map.h"
#include "datastruct/str.h"

int getopt_test(int argc, char **argv) {
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

mem_handle counts;
const unsigned int INITIAL_COUNTS_SIZE = 128;
unsigned int words_seen = 0;
map_handle mymap;

void process_line(str line) {
  str word;
  while (str_is_valid(line)) {
    line = str_split_whitespace_pop(line, &word);
    if (word.size > 0) {
      mem_handle entry = map_get(mymap, word);
      if (mem_is_valid(entry)) {
        // Seen word
        unsigned int *valp = mem_p(entry);
        ++(*valp);

      } else {
        // New word
        ++words_seen;
        if (words_seen > counts.size) {
          counts = mem_realloc(counts, counts.size * 2);
          if (!mem_is_valid(counts)) {
            puts("Error resizing counts array\n");
            exit(EXIT_FAILURE);
          }
        }
        unsigned int *arr = mem_p(counts);
        mem_handle value =
            mem_handle_from_ptr(&arr[words_seen - 1], sizeof(unsigned int));
        if (!map_set(mymap, word, value)) {
          puts("Error adding new key to map\n");
          exit(EXIT_FAILURE);
        }
        arr[words_seen - 1] = 1;
      }
    }
  }
}

void word_freq(char *fname) {
  FILE *infile;

  infile = fopen(fname, "r");
  if (infile == (void *)0) {
    printf("Could not open file '%s'\n", fname);
    exit(EXIT_FAILURE);
  }

  counts = mem_alloc(MEM_ALLOCATOR_PLAIN,
                     sizeof(unsigned int) * INITIAL_COUNTS_SIZE);
  if (!mem_is_valid(counts)) {
    puts("Error creating counts array\n");
    exit(EXIT_FAILURE);
  }

  mymap = map_create(MEM_ALLOCATOR_PLAIN);
  if (!map_is_valid(mymap)) {
    puts("Error creating map\n");
    exit(EXIT_FAILURE);
  }

  int c;
  strbuf_handle buf = strbuf_create(MEM_ALLOCATOR_PLAIN, 100);
  if (!strbuf_is_valid(buf)) {
    puts("Error creating strbuf\n");
    exit(EXIT_FAILURE);
  }
  str line;
  while ((c = getc(infile)) != EOF) {
    // This reads from stdin into a strbuf one line at a time, then uses
    // str_split_whitespace_pop to parse it into words. We could just as easily
    // do the word splitting while we read characters, but this is meant to
    // exercise the library.
    if (c == '\n') {
      line = strbuf_str(buf);
      process_line(line);
      strbuf_reset(buf);
    } else {
      if (!strbuf_concatenate_char(buf, c)) {
        puts("Error growing string buffer\n");
        exit(EXIT_FAILURE);
      }
    }
  }
  line = strbuf_str(buf);
  if (line.size > 0) process_line(line);

  int most_freq[5] = {0};
  int least_freq[5] = {0};
  unsigned int *arr = mem_p(counts);
  for (int i = 0; i < words_seen; i++) {
    unsigned int count = arr[i];
    for (int p = 0; p < 5; p++) {
      if (most_freq[p] == count) {
        break;
      }
      if (most_freq[p] < count) {
        unsigned int t = most_freq[p];
        most_freq[p] = count;
        count = t;
      }
    }
    count = arr[i];
    for (int p = 0; p < 5; p++) {
      if (least_freq[p] == count) {
        break;
      }
      if (least_freq[p] > count || least_freq[p] == 0) {
        unsigned int t = least_freq[p];
        least_freq[p] = count;
        count = t;
      }
    }
  }

  unsigned int most_freq_matches[5] = {0};
  unsigned int least_freq_matches[5] = {0};
  map_iter it = map_first_value_iter(mymap);
  while (!map_iter_done(it)) {
    mem_handle count_handle = map_iter_value(it);
    unsigned int count = *((unsigned int *)mem_p(count_handle));
    for (int i = 0; i < 5; i++) {
      if (count == most_freq[i]) {
        ++most_freq_matches[i];
      }
      if (count == least_freq[i]) {
        ++least_freq_matches[i];
      }
    }
    it = map_next_value_iter(it);
  }

  puts("Most frequent words\n==================\nCount\tWords\n");
  for (int i = 0; i < 5; i++) {
    printf("%d\t%d\n", most_freq[i], most_freq_matches[i]);
  }
  puts("\nLeast frequent words\n==================\nCount\tWords\n");
  for (int i = 0; i < 5; i++) {
    printf("%d\t%d\n", least_freq[i], least_freq_matches[i]);
  }

  fclose(infile);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    puts("Usage: m65tool file.txt");
    exit(EXIT_FAILURE);
  }
  word_freq(argv[1]);
}
