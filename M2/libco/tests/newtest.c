#include <stdio.h>
#include "co.h"

void entry(void *arg) {
  while (1) {
    printf("yeal before");
    printf("%s", (const char *)arg);
    printf("yeal after");
    co_yield();
  }
}

int main() {
  struct co *co1 = co_start("co1", entry, "a");
  struct co *co2 = co_start("co2", entry, "b");
  co_wait(co1); // never returns
  co_wait(co2);
}