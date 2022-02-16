#ifndef MIF_TEST_H_included
#define MIF_TEST_H_included

#include <utillib/cli.h>
#include <stdbool.h>

typedef struct{
    bool generate;
}mif_test_settings_t;

void mif_test_args_init(options_t *args, mif_test_settings_t *settings);
void mif_test_args_parse(options_t *args, mif_test_settings_t *settings);
bool mif_test_should_run(mif_test_settings_t *settings);
bool mif_test_run(mif_test_settings_t *settings, int argc, char **argv);

#endif
