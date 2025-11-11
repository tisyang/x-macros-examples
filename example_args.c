
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#define _ARG_CONCAT2(A, B) A ## B
#define _ARG_IF_ELSE_0(THEN, ELSE) ELSE
#define _ARG_IF_ELSE_1(THEN, ELSE) THEN
#define _ARG_IF_ELSE_IMPL(CONDITION, THEN, ELSE)    _ARG_CONCAT2(_ARG_IF_ELSE_, CONDITION)(THEN, ELSE)
#define _ARG_IF_ELSE(CONDITION, THEN, ELSE)         _ARG_IF_ELSE_IMPL(CONDITION, THEN, ELSE)

#define _ARG_IS_ALNUM_CHAR(c)       (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || ((c) >= '0' && (c) <= '9'))
#define _ARG_IS_EMPTY_STR(s)        (sizeof(s) == 1)
#define _ARG_IS_BLANK(c)            _ARG_IS_EMPTY_STR(#c)

#define _ARG_CONCAT3_IMPL(x, y, z) x##y##z
#define _ARG_CONCAT3(x, y, z) _ARG_CONCAT3_IMPL(x, y, z)
#define _ARG_SQUOTE '
#define _ARG_CHARIFY(c)  _ARG_CONCAT3(_ARG_SQUOTE, c, _ARG_SQUOTE)

#define _ARG_LIT       0
#define _ARG_INT       1
#define _ARG_FLOAT     2
#define _ARG_STR       3
#define _ARG__T(TYPE)       _ARG_##TYPE

#define _ARG__NEED_LIT      0
#define _ARG__NEED_INT      1
#define _ARG__NEED_FLOAT    1
#define _ARG__NEED_STR      1
#define _ARG__NEED(TYPE)    _ARG__NEED_##TYPE

#define _ARG_TYPE_LIT   int
#define _ARG_TYPE_INT   int
#define _ARG_TYPE_FLOAT double
#define _ARG_TYPE_STR   char *

#define _ARG_LIT_FROMSTR(x)     0
#define _ARG_INT_FROMSTR(x)     atoi(x)
#define _ARG_FLOAT_FROMSTR(x)   atof(x)
#define _ARG_STR_FROMSTR(x)     (x)

#define _ARG_LIT_FMT    "%d"
#define _ARG_INT_FMT    "%d"
#define _ARG_FLOAT_FMT  "%g"
#define _ARG_STR_FMT    "%s"

#define _ARG_FROM_STR(TYPE, s)  _ARG_##TYPE##_FROMSTR(s)
#define _ARG_PFMT(TYPE)         _ARG_##TYPE##_FMT

#define _ARG_FIELD_TYPE(TYPE)   _ARG_TYPE_##TYPE
#define _ARG_FIELD_NAME(NAME)   arg_##NAME

#define _ARG_X_STRUCT_FIELD(TYPE, SHORT, NAME, HINT, DEF, DESC) \
        _ARG_FIELD_TYPE(TYPE)  _ARG_FIELD_NAME(NAME);

#define _ARG_X_STRUCT_DEFAULT(TYPE, SHORT, NAME, HINT, DEF, DESC) \
        DEF,

#define _ARG_STRUCT_NAME(LIST)  st_##LIST##_name

#define ARG_STRUCT_TYPE(LIST) \
    struct _ARG_STRUCT_NAME(LIST) { \
        LIST(_ARG_X_STRUCT_FIELD) \
        \
        int argc;   \
        char **argv; \
        int optind; \
    }

#define ARG_STRUCT_INITIALIZER(LIST) \
    { LIST(_ARG_X_STRUCT_DEFAULT) }


#define _ARG_HAS_HINT(HINT)         (!_ARG_IS_EMPTY_STR(HINT))
#define _ARG_CHECK_HINT(TYPE, HINT) (_ARG__T(TYPE) || !_ARG_HAS_HINT(HINT))

#define _ARG_X_STATIC_CHECK(TYPE, SHORT, NAME, HINT, DEF, DESC) \
    _Static_assert(_ARG_IS_BLANK(SHORT) || _ARG_IS_ALNUM_CHAR(_ARG_CHARIFY(SHORT)), "Short opt char must be alphanumberic or left blank"); \
    _Static_assert(_ARG_CHECK_HINT(TYPE, HINT), "Datahint invalid when TYPE == LIT");


#define _ARG_X_STRUCT_PRINT(TYPE, SHORT, NAME, HINT, DEF, DESC) \
    printf("arg -%s, %s%s: ", #SHORT, #NAME, _ARG_HAS_HINT(HINT) ? "=" HINT : ""); \
    printf(_ARG_PFMT(TYPE), p_arg->_ARG_FIELD_NAME(NAME)); \
    printf(" (%s)\n", DESC);

#define ARG_STRUCT_PRINT(LIST, STRUCT_P) \
    do { \
        struct _ARG_STRUCT_NAME(LIST) *p_arg = STRUCT_P; \
        printf("argc=%d optind=%d\n", p_arg->argc, p_arg->optind); \
        LIST(_ARG_X_STRUCT_PRINT) \
    } while (0)

#define _ARG_X_PARSE_CASE(TYPE, SHORT, NAME, HINT, DEF, DESC) \
    case _ARG_CHARIFY(SHORT): _ARG_IF_ELSE(_ARG__NEED(TYPE), p_arg->_ARG_FIELD_NAME(NAME) = _ARG_FROM_STR(TYPE, optarg), p_arg->_ARG_FIELD_NAME(NAME) += 1); break;

#define _ARG_X_PARSE_OPT(TYPE, SHORT, NAME, HINT, DEF, DESC) \
    _ARG_IF_ELSE(_ARG__NEED(TYPE), #SHORT ":", #SHORT)

#define _ARG_PARSE_FUNC_NAME(LIST)  _arg_parse_##LIST

#define _ARG_PARSE_DEFINE(LIST) \
    static int _ARG_PARSE_FUNC_NAME(LIST)(int argc, char *argv[], ARG_STRUCT_TYPE(LIST) *p_arg) \
    { \
        int ret = 0; \
        int c; \
        optind = 1; \
        while ((c = getopt(argc, argv, LIST(_ARG_X_PARSE_OPT))) != -1) { \
            switch (c) { \
                LIST(_ARG_X_PARSE_CASE) \
            default: \
                ret = -1;  break; \
            } \
        } \
        p_arg->argc = argc; \
        p_arg->argv = argv; \
        p_arg->optind = optind; \
        return ret; \
    }

#define ARG_PARSE(LIST, ...)    _ARG_PARSE_FUNC_NAME(LIST)(__VA_ARGS__)

#define ARG_DEFINE(LIST) \
    LIST(_ARG_X_STATIC_CHECK) \
    _ARG_PARSE_DEFINE(LIST)

// Use Example:

// TYPE, short, long, arg hint, default, description
#define APP_ARGS_LIST(X) \
    X(LIT,   h,      help,        "",    0,  "show help") \
    X(INT,   v,   verbose, "<level>",    0,  "verbose level") \
    X(FLOAT, t,   timeout, "<secs>" ,  0.0,  "timeout secs") \
    X(STR,   f, from_file,  "<file>", NULL,  "data file")

ARG_DEFINE(APP_ARGS_LIST);

int main(int argc, char **argv)
{
    ARG_STRUCT_TYPE(APP_ARGS_LIST) arg = ARG_STRUCT_INITIALIZER(APP_ARGS_LIST);
    int ret = ARG_PARSE(APP_ARGS_LIST, argc, argv, &arg);
    ARG_STRUCT_PRINT(APP_ARGS_LIST, &arg);
    return 0;
}
