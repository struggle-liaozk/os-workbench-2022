#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct
{
    pid_t pid;
    char *name;
} process;


typedef struct tree
{
    process *data;
    struct tree *children;
    struct tree *next;
} tree;

//method
process *create_process(pid_t pid, char *name);
tree *create_tree(process *d);
tree *put_child(tree *father, tree *child);
void insert_tree(tree *root, process *cur, pid_t parent_id);
tree *delete_child(tree *head, tree *target);
void delete_peer(tree *prior, tree *cur, tree *target);
void insert_tree(tree *root, process *cur, pid_t parent_id);
void print_tree(char *prefix, tree *root, int mark_p);
void sort(tree *pre, tree *root);