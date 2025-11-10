#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#define EMPTY()
#define POSTPONE(macro_name, args) macro_name EMPTY() ( args )

#define _IF_ELSE(condition) _IF_ELSE_##condition
#define _IF_ELSE_0(then, else) else
#define _IF_ELSE_1(then, else) then

#define APP_ARGS_LIST(X) \
    X(int,      help,    h, 0,   0,      "show help") \
    X(int,      verbose, v, 1,   0,      "verbose level") \
    X(double,   timeout, t, 1,   0.0,    "timeout")


#define _STRUCT_FIELD(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
        TYPE    NAME;

#define _STRUCT_DEFAULT(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
        DEF,

#define LIST_ST_NAME(LIST)  st_##LIST##_name

#define APP_ARGS_STRUCT(LIST) \
    struct LIST_ST_NAME(LIST) { \
        LIST(_STRUCT_FIELD) \
    }

#define APP_ARGS_STRUCT_INITIALIZER(LIST) \
    { LIST(_STRUCT_DEFAULT) }

#define _OPTARGS(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
    _IF_ELSE(NEED)(#SHORT ":", #SHORT)


#define CONCAT_IMPL(x, y, z) x##y##z
#define CONCAT(x, y, z) CONCAT_IMPL(x, y, z)
#define QUOTE '
#define CHARIFY(c)  CONCAT(QUOTE, c, QUOTE)

#define CONV_int    atoi
#define CONV_double atof
#define CONV_str(x) x

#define _STRUCT_OPT(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
    case CHARIFY(SHORT): _IF_ELSE(NEED)(_args.NAME = CONV_##TYPE(optarg), _args.NAME = 1); break;

#define LIST_FUNC_NAME(LIST)  app_args_parse_##LIST
#define APP_ARGS_PARSE(LIST) \
    static APP_ARGS_STRUCT(LIST) * LIST_FUNC_NAME(LIST)(int argc, char *argv[]) \
    { \
        static struct LIST_ST_NAME(LIST) _args = APP_ARGS_STRUCT_INITIALIZER(LIST); \
        int c; \
        while ((c = getopt(argc, argv, LIST(_OPTARGS))) != -1) { \
            switch (c) { \
            LIST(_STRUCT_OPT) \
            } \
        } \
        return &_args; \
    }

#define LIST_PARSE(LIST, ...)    LIST_FUNC_NAME(LIST)(__VA_ARGS__)

#define _STRUCT_PRINT(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
    printf("arg %s=%s: ", #SHORT, #NAME); \
    printf(_Generic((TYPE){0}, int: "%d", char*: "%s", double: "%lf"), p_arg->NAME); \
    printf(" (%s)\n", TIPS);

#define PRINT_LIST_ST(LIST, STRUCT_P) \
    do { \
        struct LIST_ST_NAME(LIST) *p_arg = STRUCT_P; \
        LIST(_STRUCT_PRINT) \
    } while (0)

APP_ARGS_PARSE(APP_ARGS_LIST);

int main(int argc, char **argv)
{
    APP_ARGS_STRUCT(APP_ARGS_LIST) *arg = LIST_PARSE(APP_ARGS_LIST, argc, argv);
    PRINT_LIST_ST(APP_ARGS_LIST, arg);
    return 0;
}
