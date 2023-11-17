#ifndef TODO_H__
#define TODOS_START 5
#define MAX_MESSAGE 4096
#define MAX_NOTE 4096
typedef struct {
  int file_index, priority;
  char note[MAX_NOTE], message[MAX_MESSAGE];
  _Bool done;
} Todo;
void Todo_array_bubble_sort_priority(Todo todos[], int size);
void Todo_array_print_ncurses(Todo todos[], int size);
void Todo_swap(Todo *t1, Todo *t2);
char * Todo_file_path();
void Todo_remove_array_index(Todo todos[], int *size, int index);
unsigned int Todo_array_read_from_file(Todo todos[]);
void ncurses_clear_todos(int todos_size);
void Todo_array_write_to_file(Todo todos[], int todos_size);
void Todo_initialize(Todo *todo);
int min(int a, int b);
unsigned int Todo_array_find_index(Todo todo_array[], int size, Todo search);
#endif // TODO_H__
