
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#define _ARG_IF_ELSE(CONDITION) _ARG_IF_ELSE_##CONDITION
#define _ARG_IF_ELSE_0(THEN, ELSE) ELSE
#define _ARG_IF_ELSE_1(THEN, else) THEN

#define _ARG_DEFINE_FIELD(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
        TYPE    NAME;

#define _ARG_DEFINE_DEFAULT(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
        DEF,

#define _ARG_STRUCT_NAME(LIST)  st_##LIST##_name

#define ARG_STRUCT_TYPE(LIST) \
    struct _ARG_STRUCT_NAME(LIST) { \
        LIST(_ARG_DEFINE_FIELD) \
    }

#define _ARG_STRUCT_INITIALIZER(LIST) \
    { LIST(_ARG_DEFINE_DEFAULT) }

#define _ARG_GETOPT_SHORT(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
    _ARG_IF_ELSE(NEED)(#SHORT ":", #SHORT)


#define _ARG_CONCAT_IMPL(x, y, z) x##y##z
#define _ARG_CONCAT(x, y, z) _ARG_CONCAT_IMPL(x, y, z)
#define _ARG_SQUOTE '
#define _ARG_CHARIFY(c)  _ARG_CONCAT(_ARG_SQUOTE, c, _ARG_SQUOTE)

#define _ARG_CONV_int    atoi
#define _ARG_CONV_double atof
#define _ARG_CONV_str(x) x

#define _ARG_CASE(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
    case _ARG_CHARIFY(SHORT): _ARG_IF_ELSE(NEED)(_args.NAME = _ARG_CONV_##TYPE(optarg), _args.NAME = 1); break;

#define _ARG_PARSE_FUNC_NAME(LIST)  _arg_parse_##LIST

#define ARG_PARSE_DEFINE(LIST) \
    static ARG_STRUCT_TYPE(LIST) * _ARG_PARSE_FUNC_NAME(LIST)(int argc, char *argv[]) \
    { \
        static struct _ARG_STRUCT_NAME(LIST) _args = _ARG_STRUCT_INITIALIZER(LIST); \
        int c; \
        while ((c = getopt(argc, argv, LIST(_ARG_GETOPT_SHORT))) != -1) { \
            switch (c) { \
            LIST(_ARG_CASE) \
            } \
        } \
        return &_args; \
    }

#define ARG_PARSE(LIST, ...)    _ARG_PARSE_FUNC_NAME(LIST)(__VA_ARGS__)

#define _ARG_PRINT(TYPE, NAME, SHORT, NEED, DEF, TIPS) \
    printf("arg %s=%s: ", #SHORT, #NAME); \
    printf(_Generic((TYPE){0}, int: "%d", char*: "%s", double: "%lf"), p_arg->NAME); \
    printf(" (%s)\n", TIPS);

#define ARG_STRUCT_PRINT(LIST, STRUCT_P) \
    do { \
        struct _ARG_STRUCT_NAME(LIST) *p_arg = STRUCT_P; \
        LIST(_ARG_PRINT) \
    } while (0)


// useage:

#define APP_ARGS_LIST(X) \
    X(int,      help,    h, 0,   0,      "show help") \
    X(int,      verbose, v, 1,   0,      "verbose level") \
    X(double,   timeout, t, 1,   0.0,    "timeout")

ARG_PARSE_DEFINE(APP_ARGS_LIST);

int main(int argc, char **argv)
{
    ARG_STRUCT_TYPE(APP_ARGS_LIST) *arg = ARG_PARSE(APP_ARGS_LIST, argc, argv);
    ARG_STRUCT_PRINT(APP_ARGS_LIST, arg);
    return 0;
}
