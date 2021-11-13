#ifndef SL_TEST_H_included
#define SL_TEST_H_included

#include <utillib/cli.h>
#include <stdbool.h>

typedef struct{
    bool create;
    bool print;
    bool load_save;
    bool unpack;
}sl_settings_t;

void sl_test_args_init(options_t *args, sl_settings_t *settings);
void sl_test_args_parse(options_t *args, sl_settings_t *settings);
bool sl_test_should_run(sl_settings_t *settings);
bool sl_test_run(sl_settings_t *settings, int argc, char **argv);

#endif
