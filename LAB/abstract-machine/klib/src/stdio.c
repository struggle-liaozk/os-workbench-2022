#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>


#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  char buffer[1024];
  va_list args;
  va_start(args, fmt);

  vsnprintf(buffer, 1024, fmt, args);

  putstr(buffer);
  return 1;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char* start = out;
  while (fmt != '\0') {
    if (fmt == '%') {
      fmt++;
      switch (*fmt)
      {
      case 's':
        char *replace = va_arg(ap, char*);
        while (replace != '\0') {
          *out = *replace;
          out++;
          replace++;
        }
        break;
      case 'd':
        int d = va_arg(ap, int); 
        if (d != 0) {
          char a[32];
          int i_a = 0;
          while (d != 0) {
            char b = 48 + (d % 10);
            a[i_a] = b;
            d = d / 10;
            i_a ++;
          }
          while (i_a > 0) {
            *out = a[i_a];
            out++;
            i_a--;
          }
        } else {
          *out = 48;
        }
        break;
      default:
        panic("Not implemented this case");
        break;
      }
    } else {
      *out = *fmt;
    }
    fmt++;
    out++;
  }
  
  *out = '\0';
  return out - start;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  return vsprintf(out, fmt, args);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  return vsnprintf(out, n, fmt, args);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  size_t num = 0;
  while (fmt != '\0') {
    if (fmt == '%') {
      fmt++;
      switch (*fmt)
      {
      case 's':
        char *replace = va_arg(ap, char*);
        while (replace != '\0') {
          *out = *replace;
          out++;
          num++;
          if (num >= n) {
            *out = '\0';
            return num;
          }
          replace++;
        }
        break;
      case 'd':
        int d = va_arg(ap, int); 
        if (d != 0) {
          char a[32];
          int i_a = 0;
          while (d != 0) {
            char b = 48 + (d % 10);
            a[i_a] = b;
            d = d / 10;
            i_a ++;
          }
          while (i_a > 0) {
            *out = a[i_a];
            out++;
            num++;
            if (num >= n) {
            *out = '\0';
            return num;
            }
            i_a--;
          }
        } else {
          *out = 48;
        }
        break;
      default:
        panic("Not implemented this case");
        break;
      }
    } else {
      *out = *fmt;
    }
    fmt++;
    out++;
    num++;
    if (num >= n) {
        *out = '\0';
        return num;
    }
  }
  if (num > 0) {
    *out = '\0';
  }
  return num;
}

#endif


int main() {
  printf("test str= %s \n", "this is string");

  printf("test num= %d \n", 1024);
}
