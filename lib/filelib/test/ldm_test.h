#ifndef LDM_TEST_H_included
#define LDM_TEST_H_included

#include <utillib/cli.h>
#include <stdbool.h>

typedef struct{
    bool generate;
    bool print;
    bool load_save;
}ldm_test_settings_t;

void ldm_test_args_init(options_t *args, ldm_test_settings_t *settings);
void ldm_test_args_parse(options_t *args, ldm_test_settings_t *settings);
bool ldm_test_should_run(ldm_test_settings_t *settings);
bool ldm_test_run(ldm_test_settings_t *settings, int argc, char **argv);

#endif
