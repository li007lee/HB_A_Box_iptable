#include <setjmp.h>
#define EXC_PRINT

#ifdef EXC_PRINT
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
typedef struct except_frame_s
{
    struct except_frame_s   *prev;
    jmp_buf env;
} except_frame_t;

typedef void (*exc_func)(void *context);
typedef struct exc_handler_s
{
    exc_func    func;
    void        *context;
} exc_handler_t;


enum
{
    EXC_ENTERED,
    EXC_RAISED,
    EXC_HANDLED,
    EXC_FINALIZED
};

typedef struct except_info_s
{
    const char *fmt;
    void *p1;
    void *p2;
    void *p3;
} except_info_t;


typedef struct except_s
{
    exc_handler_t handler;
    except_info_t   info;
    except_frame_t *frame;
} except_t;


#define __try(_except) \
    do { \
        volatile int __except_flag; \
        except_frame_t __ef; \
        __ef.prev = (_except)->frame; \
        (_except)->frame = &__ef; \
        __except_flag = setjmp(__ef.env); \
        if (__except_flag == EXC_ENTERED) {

#define __finally \
        } \
        { \
            if (__except_flag == EXC_ENTERED) \
                __except_flag = EXC_FINALIZED;

#define __except \
        } else { \
            __except_flag = EXC_HANDLED;

#define __endtry(_except) \
        } \
        (_except)->frame = (_except)->frame->prev; \
        if (__except_flag == EXC_RAISED) __reraise(_except); \
    } while (0);


#define __raise(_except, _fmt, _p1, _p2, _p3) \
    __internal_raise(_except, _fmt, (void*)(_p1), (void*)(_p2), (void*)(_p3))

#define __reraise(_except) \
    __except_raise(_except)

void __internal_raise(except_t *except, const char *fmt, void *p1, void *p2, void *p3);
void __except_raise(except_t *except);
exc_handler_t exc_set_handler(except_t *except, exc_func, void *);

exc_handler_t __except_handler;

void __internal_raise(except_t *except, const char *fmt, void *p1, void *p2, void *p3);

void __except_raise(except_t *except);

exc_handler_t exc_set_handler(except_t *except, exc_func func, void *context);

