#include <unistd.h>
#include <sys/types.h>

struct process
{
    pid_t pid;
    char *name;
};


struct tree_node
{
    struct process *data;
    struct link_node *children;
};

struct link_node
{
    struct tree_node *data;
    struct link_node *next;
};

struct process *create_process(pid_t pid, char *name){
    struct process *p = (struct process *)malloc(sizeof(struct process));
    p->pid = pid;
    p->name = name;
    return p;
}

struct link_node *create_link(struct tree_node *node) {
    struct link_node *link = (struct link_node *)malloc(sizeof(struct link_node));
    link -> data = node;
    link -> next = NULL;
    return link;
}

struct tree_node *create_tree(struct process *p, struct link_node *child) {
    struct tree_node *node = (struct tree_node *)malloc(sizeof(struct tree_node));
    node -> data = p;
    // 这里的child 是一个链表
    if (child!= NULL) {
        node -> children = insert_link(node -> children, child -> data);
    } else {
        node -> children = NULL;
    }
    return node;
}

struct link_node *insert_link(struct link_node *head, struct tree_node *node) {
    struct link_node *link = create_link(node);

    if (head == NULL) {
        return link;
    }

    struct link_node *end = head;
    while (end -> next!= NULL) {
        end = end -> next;
    }

    end -> next = link;
    return head;
}

//todo 遍历找到目标目标进程


struct tree_node *find_process(struct tree_node *root, pid_t pid) {
    if (root == NULL) {
        return NULL;
    }
    if (root -> children == NULL) {
        return NULL;
    }
    struct link_node *child = root -> children;

    if (child ->data ->data -> pid == pid) {
        return child -> data;
    }
    while (child -> next!= NULL)
    {
        child = child -> next;
        struct tree_node *node = find_process(child -> data, pid);
        if (node != NULL) {
            return node;
        }
    }
    return NULL;
}

//todo 排序







