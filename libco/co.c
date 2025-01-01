#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>



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

typedef struct CONODE{
  struct co *coroutine;
  struct CONODE *f, *b;
}CONODE;

static CONODE *ALL_COROUTINE = NULL;

static void co_node_insert(struct co *newcoroutine) {
  CONODE *node = (CONODE*) malloc(sizeof(CONODE));
  assert(node);

  node -> coroutine = newcoroutine;
  if (ALL_COROUTINE) {
    node -> f = ALL_COROUTINE -> f;
    node -> b = ALL_COROUTINE;
    node -> b -> f = node -> f -> b =node;
  } else {
    node -> f = node -> b = node;
    ALL_COROUTINE = node;
  }
}

static CONODE *co_node_remove() {
  CONODE *rs = NULL;
  
  if (ALL_COROUTINE == NULL) {
    return rs;
  }
  if (ALL_COROUTINE -> b == ALL_COROUTINE) {
    rs = ALL_COROUTINE;
    ALL_COROUTINE = NULL;
  } else {
    rs = ALL_COROUTINE;
    ALL_COROUTINE = ALL_COROUTINE -> b;
    ALL_COROUTINE -> f = rs -> f;
    ALL_COROUTINE -> f -> b = ALL_COROUTINE;

  }

  return rs;
}


struct co *current; //当前正在执行的协程


/**
 *      movq 16(%%rsp),  %%rcx; movq %%rcx, 16(%0); \
     movq 8(%%rsp),  %%rcx; movq %%rcx, 8(%0); \
     movq (%%rsp),  %%rcx; movq %%rcx, 0(%0); \
 */
/**
 * "movl %%esp, 0(%0); \
     movl 4(%%esp),  %%ecx; movl %%ecx, 4(%0); \
     movl 8(%%esp),  %%ecx; movl %%ecx, 8(%0); \
     movl 12(%%esp),  %%ecx; movl %%ecx, 12(%0); \
     movl %0,  %%esp; \
     sub  $4, %%esp; \
     movl %2,  -4(%0); \
     call *%1; \
     add  $4, %%esp; "
      : : "b"((uintptr_t)sp -16), "d"(entry), "a"(arg) : "memory"
 */


//############### 以下是方法



static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %%rsp,  0(%0); \
     movq %%rdi,  8(%0); \
     movq %%rsi,  16(%0); \
     movq %%rdx,  24(%0); \
     movq %%rcx,  32(%0); \
     movq %%r8,   48(%0); \
     movq %%r9,   56(%0); \
     movq %0,  %%rsp; \
     movq %2, %%rdi; \
     call *%1;"
      : : "b"((uintptr_t)sp-64), "d"(entry), "a"(arg)  : "memory"
#else
    "movl %%esp, -0x8(%0); \
    leal -0xC(%0), %%esp; \
    movl %2, -0xC(%0); \
    call *%1; \
    movl -0x8(%0), %%esp"
		:
		: "b"((uintptr_t)sp), "d"(entry), "a"(arg)
		: "memory"
#endif
  );
}
/**
 * "movl 0(%0), %%esp; \
       movl 4(%0),  %%ecx; movl %%ecx, 4(%%esp); \
       movl 8(%0),  %%ecx; movl %%ecx, 8(%%esp); \
       movl 12(%0),  %%ecx; movl %%ecx, 12(%%esp); " 
      : :"b"((uintptr_t)sp -16)  : "memory"
 */


static inline void restore_return(void *sp) {
  asm volatile (
#if __x86_64__
			"movq 0(%0), %%rsp; \
      movq 8(%0),  %%rdi; \
      movq 16(%0), %%rsi; \
      movq 24(%0),  %%rdx; \
      movq 32(%0),  %%rcx; \
      movq 48(%0),  %%r8; \
      movq 56(%0),  %%r9; "
      : 
      : "b"((uintptr_t)sp-64) 
      : "memory"
#else
			"movl -0x8(%0), %%esp"
		:
		: "b"((uintptr_t)sp)
		: "memory"
#endif
			);
}



/*
初始化
*/
static __attribute__((constructor)) void init() {
  struct co *main= co_start("main", NULL, NULL);
  main -> status = CO_RUNNING;
  current = main;
}

/*
析构
*/
static __attribute__((destructor)) void destory() {
  if (ALL_COROUTINE == NULL) {
    return;
  }
  while (ALL_COROUTINE) {
    current = ALL_COROUTINE -> coroutine;
    free(current);
    free(co_node_remove());
  }
}




struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  //开始执行真正的start 操作
  struct co* co_s = (struct co*)malloc(sizeof(struct co));
  assert(co_s);
  co_s -> name = name;
  co_s -> func = func;
  co_s -> arg = arg;
  co_s -> status = CO_NEW;
  co_s ->waiter = NULL;

  co_node_insert(co_s);

  return co_s;
}

static void free_co(struct co *co) {
  while (ALL_COROUTINE ->coroutine != co) {
    ALL_COROUTINE = ALL_COROUTINE -> b;
  }
  free(ALL_COROUTINE -> coroutine);
  free(co_node_remove());
} 

void co_wait(struct co *co) {
  current -> status = CO_WAITING;
  co -> waiter = current;
  while (co -> status != CO_DEAD){
    co_yield();
  }
  free_co(co);
}


void co_yield() {
  int val = setjmp(current->context);
  if (val == 0) {
    //获取下一个处于co_new / co_running 状态
    while (ALL_COROUTINE -> coroutine ->status != CO_NEW || ALL_COROUTINE -> coroutine ->status != CO_RUNNING) {
      ALL_COROUTINE = ALL_COROUTINE -> b;
    }
    
    struct co* next = ALL_COROUTINE -> coroutine;
    current = next;
    debug("next name %s, next status %d \n", next -> name, next -> status);

    if (current -> status == CO_NEW) {
      ((struct co volatile*)current) -> status = CO_RUNNING;
      stack_switch_call((current -> stack + STACK_SIZE - 16), current -> func, (uintptr_t)(current -> arg));
      debug("return stcak_switch %s \n", current -> name);
      restore_return((current -> stack + STACK_SIZE - 16));
      debug("return restore_return %s \n", current -> name);
      current -> status = CO_DEAD;
      if (current -> waiter) {
        debug("change waiter status to running %s \n", current -> name);
        current -> waiter -> status = CO_RUNNING;
      }
      debug("co exec over start return %s \n", current -> name);
      co_yield();
    } else {
      longjmp(current -> context, 1);
    }
  } else {
    debug("longjmp %s \n", current -> name);
  }
  debug("co_yield return current name = %s \n", current -> name);
}


