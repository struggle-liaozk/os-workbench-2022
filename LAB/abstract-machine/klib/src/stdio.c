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
  va_end(args);
  return 1;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int rs = vsprintf(out, fmt, args);
  va_end(args);
  return rs;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int rs = vsnprintf(out, n, fmt, args);
  va_end(args);
  return rs;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  size_t len = 0;
  while (*fmt != '\0' && len < n) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt)
      {
      case 's':
        char *replace = va_arg(ap, char*);
        while (*replace != '\0') {
          *out = *replace;
          out++;
          len++;
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
          i_a--;
          while (i_a >= 0) {
            *out = a[i_a];
            out++;
            len++;
            i_a--;
          }
        } else {
          *out = 48;
          out++;
          len++;
        }
        break;
      case 'p':
        *out = 48; out ++;
        *out = 'x';out ++;
        long p = va_arg(ap, long);
        if (p != 0) {
          char a[64];
          int i_p = 0;
          while (p != 0) {
            int r = p % 16;
            char h;
            if (r < 10) {
              h = 48 + r;
            } else {
              switch (r)
              {
              case 10:
                h = 'a';
                break;
              case 11:
                h = 'b';
                break;
              case 12:
                h = 'c';
                break;
              case 13:
                h = 'd';
                break;
              case 14:
                h = 'e';
                break;
              case 15:
                h = 'f';
                break;
              default:
                break;
              }
            }

            a[i_p] = h;
            p = p / 16;
            i_p ++;
          }
          i_p--;
          while (i_p >= 0) {
            *out = a[i_p];
            out++;
            len++;
            i_p--;
          }
        } else {
          *out = 48;
          out++;
          len++;
        }
        break;
      default:
        panic("Not implemented this case");
        break;
      }
    } else {
      *out = *fmt;
      out++;
      len++;
    }
    fmt++;
  }
  
  *out = '\0';
  return len;
}

#endif