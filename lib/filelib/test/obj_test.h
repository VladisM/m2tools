#ifndef OBJ_TEST_H_included
#define OBJ_TEST_H_included

#include <utillib/cli.h>
#include <stdbool.h>

typedef struct{
    bool generate;
    bool print;
    bool load_save;
}obj_test_settings_t;

void obj_test_args_init(options_t *args, obj_test_settings_t *settings);
void obj_test_args_parse(options_t *args, obj_test_settings_t *settings);
bool obj_test_should_run(obj_test_settings_t *settings);
bool obj_test_run(obj_test_settings_t *settings, int argc, char **argv);

#endif
