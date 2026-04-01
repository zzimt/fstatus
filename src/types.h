#ifndef FSTATUS_TYPES_H_
#define FSTATUS_TYPES_H_

#include <stdbool.h>
#include <stddef.h>

typedef size_t (*ModuleUpdateFn)(const void* config, char* buf, size_t size);

typedef struct Module {
    ModuleUpdateFn update_fn;
    const void* config;
} Module;

#define GET_MODULE_CONFIG(mod_) mod_##Config

#define GET_MODULE_UPDATE(mod_) mod_##_update

#define GET_MODULE_DEFAULT_CONFIG(mod_) mod_##_DEFAULT_CONFIG

#define MODULE(name_, ...) \
    { \
        .update_fn = GET_MODULE_UPDATE(name_), \
        .config = &(const GET_MODULE_CONFIG(name_)) { GET_MODULE_DEFAULT_CONFIG(name_) __VA_ARGS__ } \
    } 

#define DEFAULTS

typedef enum FormatKind {
    FORMAT_TEXT,
    FORMAT_MODULE,
} FormatKind;

typedef struct Format {
    FormatKind kind;
    union {
        struct {
            const char* value;
            size_t len;
        } text;

        Module module;
    } val;
} Format;

#define FTEXT(text_) \
    { \
        .kind = FORMAT_TEXT, \
        .val.text.value = (text_), \
        .val.text.len = sizeof(text_) - 1, \
    }

#define FMOD(module_) \
    { \
        .kind = FORMAT_MODULE, \
        .val.module = (module_), \
    }

#endif // FSTATUS_TYPES_H_
