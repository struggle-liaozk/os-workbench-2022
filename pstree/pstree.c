#define _DEFAULT_SOURCE
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


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
  // print mark 
  printf("mark_v:%d; mark_p:%d; mark_n:%d; \n", mark_v, mark_p, mark_n);

  if (mark_v) {
    printf("great, current is our pstree program version-0.1 !\n");
    return 0;
  }

  //获取当前所有的进程信息
  DIR *dir;
  struct dirent *entry;

  dir = opendir("/proc");

  while((entry = readdir(dir))!= NULL) {
    if (entry -> d_type == DT_DIR) {
        if (entry -> d_name[0] >= '0' && entry -> d_name[0] <='9') {
          printf("porcess pid = %s \n", entry ->d_name);
          // todo 1.如何将数据保存到链表
        }
    }
  }

  //todo 2.根据链表遍历文件夹，读取关键文件获取其父id
  FILE *fp;
  char line[256];
  char ppid[64];
  char path[256];
  char *pid = "1";

  strcat(path, "/proc/");
  strcat(path, pid);
  strcat(path, "/status");

  fp = fopen(path, "r");
  while (fgets(line, sizeof(line), fp) != NULL)
  {
    if (strncmp(line, "PPid:", 5) == 0) {
      sscanf(line, "PPid:   %s", ppid);
      break;
    }
  }
  printf("parent = %s \n", ppid);
  fclose(fp);



  //todo 3.将信息装进树中


  closedir(dir);
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


