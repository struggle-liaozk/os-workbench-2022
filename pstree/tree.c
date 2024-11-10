#include "tree.h"



static tree *found_end(tree *head);
static tree *found_cur_p_pid(tree *prior, tree *root, pid_t pid);
static tree *found_pid(tree *root, pid_t pid);
static tree *split(tree *head);
static tree *merge(tree *a, tree *b);
static tree *mergesort(tree *head);


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

tree *delete_child(tree *head, tree *target) {
    if (head == target) {
        if (target->next == NULL) {
            return NULL;
        } else {
            tree * next = target ->next;
            target ->next = NULL;
            return next;
        }
    } else {
        //删除target 即可
        delete_peer(head, head->next, target);
        return head;
    }
}

void delete_peer(tree *prior, tree *cur, tree *target) {
    if (cur == NULL) return;
    if (cur == target) {
        if (target -> next == NULL) {
            prior -> next = NULL;
        } else {
            prior -> next = target -> next;
        }
    } else {
        delete_peer(cur, cur->next, target);
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
 * 0.先找树中是否存在当前pid，存在就将name更新进去。同时将其挂在正确的父节点下（先前被作为父节点创建并挂在了root下面）。
 * 1.查找树中是否存在parent
 * 2.不存在就新增一个没有name的node，挂在root下，并将当前进程挂在在该node下
 * 3.存在就将当前进程挂在node的child下
 */
void insert_tree(tree *root, process *cur, pid_t parent_id) {
    
    
    tree *target = found_pid(root, cur->pid);
    if (target != NULL) {
        //当前节点存在
        target ->data = cur;
        tree *father = found_cur_p_pid(NULL, root, cur->pid);
        assert(father);//按照我们的逻辑，一定有父节点
        father->children =  delete_child(father->children, target);
    } else {
        //当前节点不存在
        target = (tree *)malloc(sizeof(tree));
        target->data = cur;
        target->children = NULL;
        target->next = NULL;
    }
    
    //将当前节点放到合适的父节点下
    tree *parent_node = found_pid(root, parent_id);

    if (parent_node != NULL) {
        put_child(parent_node, target);
    } else {
        parent_node = (tree *)malloc(sizeof(tree));
        parent_node->data = create_process(parent_id, "");

        put_child(root, parent_node);
        put_child(parent_node, target);
    }
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

static tree *found_cur_p_pid(tree *prior, tree *root, pid_t pid) {  
    tree *rs = NULL;
    if (root->data->pid == pid) {
        return prior;
    }
    if (root->next != NULL) {
        rs = found_cur_p_pid(prior, root->next, pid);
        if (rs != NULL) {
            return rs;
        }
    }
    if (root -> children != NULL) {
        return found_cur_p_pid(root, root->children, pid);
    }

    return NULL;
}


//a final method to print tree. success is comming

void print_tree(char *prefix, tree *root, int mark_p){
    if (root -> data != NULL) {
        if (mark_p) {
            printf("%s %s(%d) \n", prefix, root->data->name, root->data->pid);
        } else {
            printf("%s %s \n", prefix, root->data->name);
        } 
    }
    if (root->children != NULL) {
        char *newprefix = (char *)malloc(strlen(prefix) + 3 +1);
        strcpy(newprefix, prefix);
        strcat(newprefix, "|--");
        print_tree(newprefix, root->children, mark_p);
    }
    if (root->next != NULL) {
        print_tree(prefix, root->next, mark_p);
    }
}


//copy 了一个归并排序
//1.split
//2.merge
//3.mergesort

// static tree *split(tree *head) {
//     tree * pre = head, *mid = head;

//     while (mid && mid->next)
//     {
//         pre = mid;
//         mid = mid->next;
//     }
//     if (pre) {
//         pre ->next =NULL;
//     } 

//     return mid;
// }

static tree *split(tree *head) {
    tree *slow = head, *fast = head, *prev = NULL;
    while (fast && fast->next) {
        prev = slow;
        slow = slow->next;
        fast = fast->next->next;
    }
    if (prev) {
        prev->next = NULL; // 切断前半部分和后半部分的链接
    }
    return slow;
}

static tree *merge(tree *a, tree *b) {
    tree *rs = NULL;
    if (a == NULL) {
        return b;
    } else if (b == NULL) {
        return a;
    }

    if (a->data->pid > b->data->pid) {
        rs = a;
        rs->next = merge(a->next, b);
    } else {
        rs = b;
        rs->next = merge(a, b->next);
    }

    return rs;
}


static tree *mergesort(tree *head) {
    if (head == NULL || head ->next == NULL) {
        return head;
    }
    tree *mid = split(head);
    tree *left = head;
    tree *right = mid;

    left = mergesort(left);
    right = mergesort(right);

    return merge(left, right);
}


void sort(tree *pre, tree *root) {
    tree *merged = NULL;
    if (root->next != NULL) {
        merged = mergesort(root);
        
        pre ->children = merged;
        tree *cur = merged;
        while (cur != NULL)
        {
            if (cur ->children != NULL) {
                sort(cur, cur->children);
            }
            
            cur = cur ->next;
        }
        
    } else {
        if (root->children != NULL) {
            sort(root, root->children);
        }
    }
    
}







/*
int main(int argc, char *argv[]) {

    // init 
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

    process *root_p11 = create_process(11, "2-11 lalala");
    tree *node11 = create_tree(root_p11);
    put_child(node2, node11);

    process *root_p12 = create_process(12, "1-12 lalala");
    tree *node12 = create_tree(root_p12);
    put_child(node1, node12);

    //test delete
    //node2->children = delete_child(node2->children, node3);
    


    // test insert
    
    process *root_p4 = create_process(4, "2-4 yeyeye");
    insert_tree(root, root_p4, 2);

    process *root_p5 = create_process(5, "1-5 yeyeye");
    process *root_p6 = create_process(6, "5-6 yeyeye");
    insert_tree(root, root_p6, 5);
    insert_tree(root, root_p5, 1);
    
    


    tree *target = found_pid(root, 11);
    tree *parent = found_cur_p_pid(NULL, root, 11);
    if (target != NULL) {
        printf("find! %s \n", target->data->name);
    } else {
        printf("find! NULL \n");
    }
    if (parent != NULL) {
        printf("find parent %s \n", parent->data->name);
    } else {
        printf("find parent NULL \n");
    }
    
    char prefix[100] = "";
    sort(NULL, root);
    print_tree(prefix ,root, 1);

    //test sort
    process *ap = create_process(1,"1");
    tree *a = create_tree(ap);

    process *bp = create_process(2,"2");
    tree *b = create_tree(bp);

    process *cp = create_process(3,"3");
    tree *c = create_tree(cp);


    a->next = c;
    c->next = b;


    tree *s = mergesort(a);
    print_tree(prefix ,s, 1);    

    return 0;
}
*/









