#define _DEFAULT_SOURCE
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "tree.h"


void mark(int *v,int *p, int *n, char *param);





int main(int argc, char *argv[]) {

  int mark_v = 0;
  int mark_p = 0;
  int mark_n = 0;

  for (int i = 1; i < argc; i++) {
    assert(argv[i]);
    char param;
    sscanf(argv[i],"-%c", &param);
    mark(&mark_v, &mark_p, &mark_n, &param);
  }

  if (mark_v) {
    printf("great, current is our pstree program version-0.1 !\n");
    return 0;
  }

  //获取当前所有的进程信息
  DIR *dir;
  struct dirent *entry;
  dir = opendir("/proc");


  //tree 
  process *root_p = create_process(0, "root");
  tree *root = create_tree(root_p);


  while((entry = readdir(dir))!= NULL) {
    if (entry -> d_type == DT_DIR) {
        if (entry -> d_name[0] >= '0' && entry -> d_name[0] <='9') {
          //获取内部结构相关内容
          FILE *fp;
          char line[256];
          char ppid_s[8];
          char name[128];
          char path[128];
          char *pid_s = entry ->d_name;

          for (int i = 0; i < strlen(path); i++) {
            path[i]= '\0';
          }
          strcat(path, "/proc/");
          strcat(path, pid_s);
          strcat(path, "/status");

          fp = fopen(path, "r");
          while (fgets(line, sizeof(line), fp) != NULL)
          {
            if (strncmp(line, "Name:", 5) == 0) {
              sscanf(line, "Name:   %s", name);
            }
            if (strncmp(line, "PPid:", 5) == 0) {
              sscanf(line, "PPid:   %s", ppid_s);
              break;
            }
          }
          pid_t pid = atoi(pid_s);
          pid_t ppid = atoi(ppid_s);
          char *pname = (char *)malloc(sizeof(char) *128);
          strcpy(pname, name);
          process *cur = create_process(pid, pname);
          insert_tree(root, cur, ppid);

          fclose(fp);

        }
    }
  }
  closedir(dir);

  if (mark_n) {
    //todo 排序
  }
  print_tree("", root, mark_p);

  return 0;
}


void mark(int *v,int *p, int *n, char *param) {
  switch (*param)
  {
  case 'V':
    *v = 1;
    break;
  case 'p':
    *p = 1;
    break;
  case 'n':
    *n = 1;
    break;  
  default:
    break;
  }
}


