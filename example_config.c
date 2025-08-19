// build: gcc test.c -o test $(pkg-config --cflags --libs libcjson)
//
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <cjson/cJSON.h>

#define CFG_STR_MAXCHARS    32

#define LTS_CFG_CONTROL_LIST(X) \
    X(STR, serial, CFG_STR_MAXCHARS, "/dev/ttyS4", "control serial device", true) \
    X(INT, baudrate, 460800, "control serial baudrate", false) \
    X(FLOAT, scan_speed, 5.0, "scan speed deg/s", false)

#define LIDAR_ROT_DEFAULT ((double[]){ 0, 0, -1, -1, 0, 0, 0, 1, 0 })

#define LTS_CFG_LIDAR_LIST(X) \
    X(STR, ip, CFG_STR_MAXCHARS, "192.168.0.240", "lidar ip addr", false) \
    X(INT, port, 2111, "lidar port", false) \
    X(FLOAT_N, p_rot, 9, LIDAR_ROT_DEFAULT, "rot matrix", true)

#define LTS_CFG_CAMERA_LIST(X) \
    X(STR, device, CFG_STR_MAXCHARS, "/dev/video0", "camera device", false) \
    X(INT, width, 1920, "camera frame width", false) \
    X(INT, height, 1080, "camera frame height", false)

#define LTS_CFG_SECTIONS_LIST(SECTION, X) \
    SECTION(control,  LtsConfigControl,   LTS_CFG_CONTROL_LIST(X)) \
    SECTION(lidar,    LtsConfigLidar,     LTS_CFG_LIDAR_LIST(X)) \
    SECTION(camera,   LtsConfigCamera,    LTS_CFG_CAMERA_LIST(X))

#define X_INT__STDEF(N, DEF, DESC, REQD)               int     N;
#define X_FLOAT__STDEF(N, DEF, DESC, REQD)             double  N;
#define X_FLOAT_N__STDEF(N, L, DEF, DESC, REQD)        double  N[L];
#define X_STR__STDEF(N, L, DEF, DESC, REQD)            char    N[L + 1];

#define X_STDEF(TYPE_MACRO, N, ...) X_##TYPE_MACRO##__STDEF(N, __VA_ARGS__)

#define X_SECTION__STRUCT(name, stname, list) \
    struct stname { \
        list \
    } name;

struct LtsConfig {
    LTS_CFG_SECTIONS_LIST(X_SECTION__STRUCT, X_STDEF)
};

#define X_INT__INIT(N, DEF, DESC, REQD)                 cfg->N = DEF;
#define X_FLOAT__INIT(N, DEF, DESC, REQD)               cfg->N = DEF;
#define X_FLOAT_N__INIT(N, L, DEF, DESC, REQD)          memcpy(cfg->N, DEF, sizeof(cfg->N));
#define X_STR__INIT(N, L, DEF, DESC, REQD)              snprintf(cfg->N, sizeof(cfg->N), "%s", DEF);

#define X_INIT(TYPE_MACRO, N, ...) X_##TYPE_MACRO##__INIT(N, __VA_ARGS__)

#define X_SECTION__INIT(name, stname, list) \
    { \
        struct stname *cfg = &config->name; \
        list \
    }

static inline void lts_config_init(struct LtsConfig *config)
{
    LTS_CFG_SECTIONS_LIST(X_SECTION__INIT, X_INIT)
}

#define X_INT__PRINT(N, DEF, DESC, REQD)      fprintf(fp, "  %-10s = %d\n", #N, sec->N);
#define X_FLOAT__PRINT(N, DEF, DESC, REQD)    fprintf(fp, "  %-10s = %f\n", #N, sec->N);
#define X_FLOAT_N__PRINT(N, L, DEF, DESC, REQD) \
    { \
        fprintf(fp, "  %-10s = [", #N); \
        for (size_t i = 0; i < (L); i++) { \
            fprintf(fp, "%f%s", sec->N[i], (i + 1 < (L)) ? ", " : ""); \
        } \
        fprintf(fp, "]\n"); \
    }
#define X_STR__PRINT(N, L, DEF, DESC, REQD)   fprintf(fp, "  %-10s = \"%s\"\n", #N, sec->N);
#define X_PRINT(TYPE_MACRO, N, ...) X_##TYPE_MACRO##__PRINT(N, __VA_ARGS__)

#define X_SECTION__PRINT(name, stname, list) \
    { \
        const struct stname *sec = &config->name; \
        fprintf(fp, "[%s]\n", #name); \
        list \
        fprintf(fp, "\n"); \
    }

static inline void lts_config_fprint(FILE *fp, const struct LtsConfig *config)
{
    LTS_CFG_SECTIONS_LIST(X_SECTION__PRINT, X_PRINT)
}


#define X_INT__JSON(N, DEF, DESC, REQD) \
    cJSON_AddNumberToObject(sec_json, #N, sec->N);
#define X_FLOAT__JSON(N, DEF, DESC, REQD) \
    cJSON_AddNumberToObject(sec_json, #N, sec->N);
#define X_FLOAT_N__JSON(N, L, DEF, DESC, REQD) \
    { \
        cJSON *arr = cJSON_CreateArray(); \
        for (size_t i = 0; i < (L); i++) { \
            cJSON_AddItemToArray(arr, cJSON_CreateNumber(sec->N[i])); \
        } \
        cJSON_AddItemToObject(sec_json, #N, arr); \
    }
#define X_STR__JSON(N, L, DEF, DESC, REQD) \
    cJSON_AddStringToObject(sec_json, #N, sec->N);

#define X_JSON(TYPE_MACRO, N, ...) X_##TYPE_MACRO##__JSON(N, __VA_ARGS__)
#define X_SECTION__JSON(name, stname, list) \
    { \
        const struct stname *sec = &config->name; \
        cJSON *sec_json = cJSON_CreateObject(); \
        list \
        cJSON_AddItemToObject(root, #name, sec_json); \
    }

static inline char *lts_config_dump_json(const struct LtsConfig *config)
{
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;

    LTS_CFG_SECTIONS_LIST(X_SECTION__JSON, X_JSON)

    char *json_str = cJSON_Print(root);
    cJSON_Delete(root);
    return json_str;
}

int main(int argc, char *argv)
{
    struct LtsConfig config;
    lts_config_init(&config);
    lts_config_fprint(stdout, &config);
    char *json = lts_config_dump_json(&config);
    printf("JSON len=%zu:\n %s\n", strlen(json), json);
    free(json);
    return 0;
}
