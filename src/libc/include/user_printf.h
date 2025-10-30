#include <stdarg.h>

void tfp_format_user(void* putp,void (*putf) (void*,char),char *fmt, va_list va);
void tfp_sprintf_u(char* s,char *fmt, ...);
