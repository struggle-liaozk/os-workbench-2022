#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>



#ifdef LOCAL_MACHINE
  #define debug(...) printf(__VA_ARGS__)
#else
  #define debug(...)
#endif

#define STACK_SIZE  (1024 * 128) + 16 //假定stack 不会超过128KB


enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};

struct co {
  const char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程；需要当前一结束就执行等待协程吗？
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
};

struct co* ALL_CO[128 + 1]; //假定不会超过128个协程
static uint8_t ALL_CUR_MAX = 0; //协程数组的
static uint8_t ALL_CUR_RAND = 0;


struct co *current; //当前正在执行的协程


/**
 *      movq 16(%%rsp),  %%rcx; movq %%rcx, 16(%0); \
     movq 8(%%rsp),  %%rcx; movq %%rcx, 8(%0); \
     movq (%%rsp),  %%rcx; movq %%rcx, 0(%0); \
 */


//############### 以下是方法



static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %%rsp,  0(%0); \
     movq %0,  %%rsp; \
     movq %2, %%rdi; \
     call *%1; "
      : : "b"((uintptr_t)sp), "d"(entry), "a"(arg)  : "memory"
#else
    "movl %%esp, 0(%0); \
     movl %0,  %%esp; \
     movl %2,  4(%0); \
     call *%1"
      : : "b"((uintptr_t)sp - 4), "d"(entry), "a"(arg) : "memory"
#endif
  );
}


static inline void restore_return(void *sp) {
  asm volatile (
#if __x86_64__
			"movq 0(%0), %%rsp;" : : "b"((uintptr_t)sp) : "memory"
#else
			"movl 0(%0), %%esp;" : :  "b"((uintptr_t)sp - 4) : "memory"
#endif
			);
}








struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  if (ALL_CUR_MAX == 0) {
    //初始化main 协程，并放在列表的开头位置
    struct co* co_main = (struct co*)malloc(sizeof(struct co));
    co_main -> name = "main";
    co_main -> status = CO_RUNNING;
    ALL_CO[0] = co_main;
    ALL_CUR_MAX ++;
    current = co_main;
  }

  //开始执行真正的start 操作
  struct co* co_s = (struct co*)malloc(sizeof(struct co));
  co_s -> name = name;
  co_s -> func = func;
  co_s -> arg = arg;
  co_s -> status = CO_NEW;

  ALL_CO[ALL_CUR_MAX] = co_s;
  ALL_CUR_MAX ++;

  return co_s;
}

void free_co(struct co* co){
  uint8_t cur_index = 0;
    int i = 1;
    for (i; i < ALL_CUR_MAX; i ++) {

      struct co* cur = ALL_CO[i];
      if (cur == co) {
        cur_index = i;
      }
      if (i >= cur_index) {
        ALL_CO[i] = ALL_CO[i + 1];
      }
    }
    ALL_CUR_MAX --;
    //回收
    free(co);
    debug("wait free %d \n", i);
    debug("ALL_CUR_MAX %d \n", ALL_CUR_MAX);
}


void co_wait(struct co *co) {
  if (co -> status == CO_DEAD) {
    debug("should never arrive wait yield return %s \n", "h");
    free_co(co);
  } else {
    current -> status = CO_WAITING;
    co -> waiter = current;
    co_yield();
    debug("wait yield return %s \n", "h");
    free_co(co);
    current = ALL_CO[0];
  }  
}


void co_yield() {
  int val = setjmp(current->context);
  if (val == 0) {
    //从容器中x随机选一个，longjmp
    ALL_CUR_RAND = (ALL_CUR_RAND + 1) % ALL_CUR_MAX;
    //uint8_t next_index = ALL_CUR_MAX % 2;

    
    debug("next_index = %d,ALL_CUR_MAX = %d\n", ALL_CUR_RAND, ALL_CUR_MAX);
    
    struct co* next = ALL_CO[ALL_CUR_RAND];
    current = next;

    switch (next -> status)
    {
    case CO_NEW:
      next -> status = CO_RUNNING;
      stack_switch_call(&(next -> stack[STACK_SIZE - 16]), next -> func, (uintptr_t)(next -> arg));
      debug("return %s \n", "stcak_switch");
      restore_return(&(next -> stack[STACK_SIZE - 16]));
      debug("return %s \n", "restore_return");
      next -> status = CO_DEAD;
      debug("co_new return %s \n", "a");
      if (next -> waiter) {
        next -> waiter -> status = CO_RUNNING;
      }
      break; 
    case CO_RUNNING:
      longjmp(next -> context, 1);
      debug("co_running return %s \n", "b");
      break;
    case CO_WAITING:
      co_yield();
      debug("co_wait return %s \n", "c");
      break;
    case CO_DEAD:
      debug("should never arrive %s \n", "d");
      co_yield();
      break;
    }
    debug("end %s \n","e");

  } else {
    //继续执行当前协程
    debug("longjmp %s \n", "f");
    printf("%s", "longjmp \n");
    return;
  }

  debug("over %s \n", "g");

}


