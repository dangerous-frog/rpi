
// Small printf bandaid
#include "user_printf.h"


typedef void (*putcf) (void*,char);
static putcf stdout_putf;


static void ui2a_u_u(unsigned int num, unsigned int base, int uc,char * bf)
    {
    int n=0;
    unsigned int d=1;
    while (num/d >= base)
        d*=base;
    while (d!=0) {
        int dgt = num / d;
        num%= d;
        d/=base;
        if (n || dgt>0 || d==0) {
            *bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
            ++n;
            }
        }
    *bf=0;
    }


    static void putchw_u(void* putp,putcf putf,int n, char z, char* bf)
    {
    char fc=z? '0' : ' ';
    char ch;
    char* p=bf;
    while (*p++ && n > 0)
        n--;
    while (n-- > 0)
        putf(putp,fc);
    while ((ch= *bf++))
        putf(putp,ch);
    }

static void i2a_u (int num, char * bf)
    {
    if (num<0) {
        num=-num;
        *bf++ = '-';
        }
    ui2a_u_u(num,10,0,bf);
    }

static int a2d_u(char ch)
    {
    if (ch>='0' && ch<='9')
        return ch-'0';
    else if (ch>='a' && ch<='f')
        return ch-'a'+10;
    else if (ch>='A' && ch<='F')
        return ch-'A'+10;
    else return -1;
    }

static char a2i_u(char ch, char** src,int base,int* nump)
    {
    char* p= *src;
    int num=0;
    int digit;
    while ((digit=a2d_u(ch))>=0) {
        if (digit>base) break;
        num=num*base+digit;
        ch=*p++;
        }
    *src=p;
    *nump=num;
    return ch;
    }



void tfp_format_user(void* putp,putcf putf,char *fmt, va_list va)
    {
    char bf[12];

    char ch;


    while ((ch=*(fmt++))) {
        if (ch!='%')
            putf(putp,ch);
        else {
            char lz=0;
#ifdef  PRINTF_LONG_SUPPORT
            char lng=0;
#endif
            int w=0;
            ch=*(fmt++);
            if (ch=='0') {
                ch=*(fmt++);
                lz=1;
                }
            if (ch>='0' && ch<='9') {
                ch=a2i_u(ch,&fmt,10,&w);
                }
#ifdef  PRINTF_LONG_SUPPORT
            if (ch=='l') {
                ch=*(fmt++);
                lng=1;
            }
#endif
            switch (ch) {
                case 0:
                    goto abort;
                case 'u' : {
#ifdef  PRINTF_LONG_SUPPORT
                    if (lng)
                        uli2a_u(va_arg(va, unsigned long int),10,0,bf);
                    else
#endif
                    ui2a_u_u(va_arg(va, unsigned int),10,0,bf);
                    putchw_u(putp,putf,w,lz,bf);
                    break;
                    }
                case 'd' :  {
#ifdef  PRINTF_LONG_SUPPORT
                    if (lng)
                        li2a_u(va_arg(va, unsigned long int),bf);
                    else
#endif
                    i2a_u(va_arg(va, int),bf);
                    putchw_u(putp,putf,w,lz,bf);
                    break;
                    }
                case 'x': case 'X' :
#ifdef  PRINTF_LONG_SUPPORT
                    if (lng)
                        uli2a_u(va_arg(va, unsigned long int),16,(ch=='X'),bf);
                    else
#endif
                    ui2a_u_u(va_arg(va, unsigned int),16,(ch=='X'),bf);
                    putchw_u(putp,putf,w,lz,bf);
                    break;
                case 'c' :
                    putf(putp,(char)(va_arg(va, int)));
                    break;
                case 's' :
                    putchw_u(putp,putf,w,0,va_arg(va, char*));
                    break;
                case '%' :
                    putf(putp,ch);
                default:
                    break;
                }
            }
        }
    abort:;
    }


static void putcp_u(void* p,char c)
    {
    *(*((char**)p))++ = c;
    }


void tfp_sprintf_u(char* s,char *fmt, ...)
    {
    va_list va;
    va_start(va,fmt);
    tfp_format_user(&s,putcp_u,fmt,va);
    putcp_u(&s,0);
    va_end(va);
    }