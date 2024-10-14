#include <cmatrix.h>
#include <stddef.h>
#include <stdlib.h>
#include <terminal.h>
#include <vga.h>

#ifdef TEST_KPRINTF_S_PAT
#define print_char(info_ptr, backgroud, foreground)                 \
  do {                                                              \
    struct colored_char char_state = {                              \
        .bg_color = backgroud,                                      \
        .fg_color = foreground,                                     \
        .code = info_ptr->head->character,                          \
        .pos = TERMINAL_POS(info_ptr->head->row, info_ptr->column), \
    };                                                              \
    kprintf(0, "%s", char_state);                                   \
  } while (0)
#else
#define print_char(info_ptr, background, foreground)                       \
  do {                                                                     \
    kprintf(TERMINAL_POS(info_ptr->head->row, info_ptr->column), "%b%f%c", \
            background, foreground, info_ptr->head->character);            \
  } while (0)
#endif

typedef struct cmatrix_node {
  u8 row;
  u8 character;
  struct cmatrix_node *pred;
  struct cmatrix_node *succ;
} cm_node;

typedef struct cmatrix_info {
  u8 column;
  u8 max_length;
  u8 now_length;
  struct cmatrix_node *head;
  struct cmatrix_node *tail;
} cm_info;

#define NODE_POOL_SIZE ((TERMINAL_ROW) * (TERMINAL_COLUMN))
static cm_node node_pool[NODE_POOL_SIZE];

#define INFO_POOL_SIZE TERMINAL_COLUMN
static cm_info info_pool[INFO_POOL_SIZE];

const u8 char_pool[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '!', '@', '#', '$', '^', '&', '*', '(', ')', '{', '}', '[', ']',
    '|', '?', '/', ':', ';', '`', '~', '>', '<', ',', '.',
};

static cm_node *alloc_node() {
  for (int i = 0; i < NODE_POOL_SIZE; ++i) {
    if (node_pool[i].character == 0) {
      return &node_pool[i];
    }
  }
  return NULL;
}

static void free_node(cm_node *node_ptr) {
  node_ptr->row = 0;
  node_ptr->character = 0;
  if (node_ptr->pred != NULL) {
    node_ptr->pred->succ = NULL;
    node_ptr->pred = NULL;
  }
  if (node_ptr->succ != NULL) {
    node_ptr->succ->pred = NULL;
    node_ptr->succ = NULL;
  }
}

static void init_info(cm_info *info_ptr) {
  info_ptr->max_length = rand() % (TERMINAL_ROW - 1) + 1;
  info_ptr->head = info_ptr->tail = NULL;
}

static void expand_info(cm_info *info_ptr) {
  u8 character = char_pool[rand() % lengthof(char_pool)];

  if (info_ptr->head == NULL) {
    cm_node node = {
        .row = 0,
        .character = character,
        .pred = NULL,
        .succ = NULL,
    };
    cm_node *node_ptr = alloc_node();
    *node_ptr = node;
    info_ptr->head = node_ptr;
    info_ptr->tail = node_ptr;
    ++info_ptr->now_length;
    print_char(info_ptr, GREEN, WHITE);
  } else if (info_ptr->head->row < TERMINAL_ROW - 1) {
    cm_node node = {
        .row = info_ptr->head->row + 1,
        .character = character,
        .pred = NULL,
        .succ = info_ptr->head,
    };
    cm_node *node_ptr = alloc_node();
    *node_ptr = node;
    print_char(info_ptr, BLACK, GREEN);
    info_ptr->head->pred = node_ptr;
    info_ptr->head = node_ptr;
    ++info_ptr->now_length;
    print_char(info_ptr, GREEN, WHITE);
  } else {
    print_char(info_ptr, BLACK, GREEN);
  }

  if (info_ptr->now_length > info_ptr->max_length ||
      info_ptr->head->row == TERMINAL_ROW - 1) {
    cm_node *node_ptr = info_ptr->tail;
    kprintf(TERMINAL_POS(info_ptr->tail->row, info_ptr->column), "%c", ' ');
    info_ptr->tail = info_ptr->tail->pred;
    --info_ptr->now_length;
    free_node(node_ptr);
  }
}

void cmatrix() {
  term_clear();
  for (int i = 0; i < TERMINAL_COLUMN; ++i) {
    info_pool[i].column = i;
  }
  while (1) {
    for (int i = 0; i < TERMINAL_COLUMN; ++i) {
      cm_info *info_ptr = &info_pool[i];
      if (info_ptr->now_length == 0) {
        init_info(info_ptr);
      }
      expand_info(info_ptr);
    }
    busy_delay(1);
  }
}
