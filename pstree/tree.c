#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

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
static tree *found_end(tree *head);



process *create_process(pid_t pid, char *name){
    process *p = (process *)malloc(sizeof(process));
    p->pid = pid;
    p->name = name;
    return p;
}

tree *create_tree(process *d) {
    tree *t = (tree *)malloc(sizeof(tree));
    t->data = d;
    t->children = NULL;
    t->next = NULL;

    return t;
}

tree *put_child(tree *father, tree *child) {
    if (father -> children == NULL) {
        father->children = child;
        return child;
    } else {
        tree * end = found_end(father ->children);
        end ->next = child;
    }
}

static tree *found_end(tree *head) {
    if (head->next == NULL) {
        return head;
    } else {
        return found_end(head->next);
    }
}


//最重要的问题，如何构建这棵树？
/**
 * 0.先找树中是否存在当前pid，存在就将name更新进去。
 * 1.查找树中是否存在parent
 * 2.不存在就新增一个没有name的node，挂在root下，并将当前进程挂在在该node下
 * 3.存在就将当前进程挂在node的child下
 */
void insert_tree(tree *root, process cur, pid_t parent) {
    
}

static tree *found_pid(tree *root, pid_t pid) {
    tree * rs = NULL;
    if (root -> data ->pid == pid) {
        return root;
    } 
    if (root -> next != NULL) {
        rs = found_pid(root->next, pid);
        if (rs != NULL) {
            return rs;
        }
    }
    if (root ->children != NULL) {
        return found_pid(root->children, pid);
    }

    return NULL;
}



int main(int argc, char *argv[]) {

    process *root_p = create_process(0, "systemd");
    tree *root = create_tree(root_p);

    process *root_p1 = create_process(1, "0-1");
    tree *node1 = create_tree(root_p1);
    root->children = node1;

    process *root_p2 = create_process(2, "0-2");
    tree *node2 = create_tree(root_p2);
    node1->next = node2;

    process *root_p3 = create_process(3, "2-3 lalala");
    tree *node3 = create_tree(root_p3);
    node2->children = node3;


    tree *target = found_pid(root, 3);
    printf("find! %s \n", target->data->name);

    return 0;
}








