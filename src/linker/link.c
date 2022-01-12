#include "link.h"

#include "common.h"
#include "ldparser.h"
#include "cache.h"

#include <utillib/core.h>
#include <filelib.h>
#include <platformlib.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static ldm_memory_t *find_ldm_memory_by_name(ldm_file_t *ldm, char *name){
    for(unsigned int i = 0; i < list_count(ldm->memories); i++){
        ldm_memory_t *ldm_mem = NULL;
        list_at(ldm->memories, i, (void *)&ldm_mem);

        if(strcmp(ldm_mem->memory_name, name) == 0){
            return ldm_mem;
        }
    }

    return NULL;
}

static bool linker_assing_memories(lds_t *lds, ldm_file_t *ldm, cache_t *cache){
    CHECK_NULL_ARGUMENT(lds);
    CHECK_NULL_ARGUMENT(ldm);
    CHECK_NULL_ARGUMENT(cache);

    //go thru all memories from lds and all assigned sections
    for(unsigned int memories_index = 0; memories_index < list_count(lds->memories); memories_index++){
        mem_t *lds_mem = NULL;
        list_at(lds->memories, memories_index, (void *)&lds_mem);

        for(unsigned int section_index = 0; section_index < list_count(lds_mem->sections); section_index++){
            char *section_name = NULL;
            list_at(lds_mem->sections, section_index, (void *)&section_name);

            ldm_memory_t *ldm_mem = find_ldm_memory_by_name(ldm, lds_mem->name);

            if(ldm_mem == NULL){
                error("Error, memory from lds was not created in ldm!"); //non return function
            }

            cache_assing_section_into_memory(cache, section_name, ldm_mem);
        }
    }

    return true;
}

bool linker_link(void){
    lds_t *lds = NULL;
    cache_t *cache = NULL;
    ldm_file_t *ldm = NULL;

    LOG_MSG("Linking...");

    platformlib_init();
    filelib_init();

    if(!parse_lds(settings.input.linker_script, &lds)){
        LOG_MSG("Parse LDS - FAIL");
        return false;
    }

    LOG_MSG("Parse LDS - OK");

    ldm_file_new(&ldm);

    for(unsigned int i = 0; i < list_count(lds->memories); i++){
        mem_t *head_mem = NULL;
        ldm_memory_t *tmp_mem = NULL;

        list_at(lds->memories, i, (void *)&head_mem);

        ldm_mem_new(head_mem->name, &tmp_mem, head_mem->size, head_mem->orig);
        ldm_mem_into_file(ldm, tmp_mem);
    }

    LOG_MSG("Created LDM struct");

    cache_new(&cache);

    for(unsigned int i = 0; i < list_count(settings.input.input_obj_files); i++){
        char *filename = NULL;
        list_at(settings.input.input_obj_files, i, (void *)&filename);

        if(!cache_load_object_file(cache, filename)){
            LOG_MSG("Loading input obj files - FAIL");
            return false;
        }
    }

    for(unsigned int i = 0; i < list_count(settings.input.input_sl_files); i++){
        char *filename = NULL;
        list_at(settings.input.input_sl_files, i, (void *)&filename);

        if(!cache_load_library_file(cache, filename)){
            LOG_MSG("Loading input sl files - FAIL");
            return false;
        }
    }

    LOG_MSG("Loading input files - OK");
    if(settings.verbose == true) print_cache(cache);

    if(!cache_build_symbol_table(cache, lds->symbols, lds->entry_point)){
        LOG_MSG("Building symbol table - FAIL");
        return false;
    }

    LOG_MSG("Building symbol table - OK");
    if(settings.verbose == true) print_cache(cache);

    if(settings.strip_unused == true){
        cache_strip_down_unused_sections(cache);
        LOG_MSG("Stripped down unused sections - OK");
        if(settings.verbose == true) print_cache(cache);
    }

    if(!linker_assing_memories(lds, ldm, cache)){
        LOG_MSG("Memory assing - FAIL");
        return false;
    }

    LOG_MSG("Memory assign - OK");
    if(settings.verbose == true) print_cache(cache);

    cache_calculate_real_exported_addresses(cache);

    LOG_MSG("Exported addresses - OK");
    if(settings.verbose == true) print_cache(cache);

    if(!cache_evaluate_labels(cache, ldm)){
        LOG_MSG("Symbol eval - FAIL");
        return false;
    }

    LOG_MSG("Symbol eval - OK");
    if(settings.verbose == true) print_cache(cache);

    if(!cache_relocate_data(cache)){
        LOG_MSG("Relocation - FAIL");
        return false;
    }

    LOG_MSG("Relocation - OK");
    if(settings.verbose == true) print_cache(cache);

    if(!cache_link_specials(cache)){
        LOG_MSG("Linking - FAIL");
        return false;
    }

    LOG_MSG("Linking - OK");
    if(settings.verbose == true) print_cache(cache);

    cache_write_data_into_associated_ldm(cache);

    cache_destroy(cache);
    cache = NULL;

    if(!ldm_write(ldm, settings.output_filename)){
        LOG_MSG("Writing LDM - FAIL");
        return false;
    }

    LOG_MSG("Writing LDM - OK");

    ldm_file_destroy(ldm);
    ldm = NULL;

    free_lds(lds);
    lds = NULL;

    platformlib_deinit();
    filelib_deinit();

    return true;
}
