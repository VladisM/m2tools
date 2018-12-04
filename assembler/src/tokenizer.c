#include <tokenizer.h>

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TOKEN_EVAL_BUFF_MAX 160

/* -----------------------------------------------------------------------------
 * Type definitions
 */

typedef enum{
    IDLE,
    COMMENT,
    TOK
}tokState_t;

typedef struct fileInfo_s{
    char *absName;
    char *absPath;
    char *name;
}fileInfo_t;

typedef struct def_s{
    char *constant;
    struct def_s *next;
}def_t;

typedef struct cons_s{
    char *constant;
    char *value;
    struct cons_s *next;
}cons_t;

typedef enum{
    EVAL_NONE,
    EVAL_PSEUD,
    EVAL_LABEL,
    EVAL_PREPR,
    EVAL_INSTR
}eval_tok_type;

/* -----------------------------------------------------------------------------
 * Private functions declarations
 */

static void __tokenizer(char * filename);

static const char* is_pseudo(char *s);
static const char* is_prepr(char * s);

static void exec_prepr_cmd(char * line, fileInfo_t * fileInfo, unsigned int *false_if);

static void eval_tok(char * token, char separator, fileInfo_t * fileInfo, unsigned int line);
static void append_token(char * token);
static void clean_eval_buff(void);
static int is_string(char * token);
static int is_numero(char * token);

static void eval_pseudo(char * token, fileInfo_t * fileInfo, unsigned int line, unsigned int *false_if);
static void eval_label(char * token, fileInfo_t * fileInfo, unsigned int line, unsigned int *fasle_if);
static void eval_instr(char * token, fileInfo_t * fileInfo, unsigned int line, unsigned int *fasle_if);
static void append_to_toklist(tok_t *x);

static void save_def(char * s);
static int is_def(char *s);
static void save_cons(char * s, char * v);
static char* is_cons(char * s);

static void care_about_filelist(fileInfo_t *fileInfo);
static fileInfoOut_t * get_current_fileInfoOut(char *fileNameAbs);
static inline fileInfoOut_t * make_new_fileitem(fileInfo_t *fileInfo);
static int is_in_list(char *absName);

/* -----------------------------------------------------------------------------
 * Variables
 */

def_t *def_list = NULL;         //list for definitions
cons_t *cons_list = NULL;       // list for constants

fileInfoOut_t *filelist_first = NULL;
fileInfoOut_t *filelist_last = NULL;

tok_t *toklist_first = NULL;
tok_t *toklist_last = NULL;

eval_tok_type actual_eval = EVAL_NONE;

unsigned int iteration = 0;
const char * format_string;   //string returned by various functions like is_pseudo()
char token_eval_buff[TOKEN_EVAL_BUFF_MAX];
unsigned int token_eval_buff_top = 0;

unsigned int false_if_count = 0;


/* -----------------------------------------------------------------------------
 * Exported functions definitions
 */

void tokenizer(char * filename){
    __tokenizer(filename);
}

/* -----------------------------------------------------------------------------
 * Tokenizing functions
 */

static void __tokenizer(char * filename){
    FILE *fp;                      //pointer to file
    char *abs_filename;            //absolute filename
    char *abs_filename_path, *abs_filename_path_copy;
    char *filename_name, *filename_name_copy;

    tokState_t tokState = IDLE;     //state of tokenizer

    int c = 0;                      //variable to store char in reading loop
    unsigned int lineCounter = 0;   //line counter
    char tok_buff[TOKEN_EVAL_BUFF_MAX];//simple buffer for storing token
    int tok_buff_top = 0;           //"pointer" to the top of the buffer

    fileInfo_t fileInfo;

    if(filename == NULL){
        fprintf(stderr, "Tokenizer have NULL pointer instead filename!\n");
        exit(EXIT_FAILURE);
    }

    //get absolute path of the file and path itself
    abs_filename = realpath(filename, NULL);

    if(abs_filename == NULL){
        fprintf(stderr, "Failed to find real path to file '%s'.\n", filename);
        exit(EXIT_FAILURE);
    }

    abs_filename_path_copy = (char *)malloc(sizeof(char) * (strlen(abs_filename) + 1));
    filename_name_copy = (char *)malloc(sizeof(char) * (strlen(abs_filename) + 1));

    if(abs_filename_path_copy == NULL || filename_name_copy == NULL){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    strcpy(abs_filename_path_copy, abs_filename);
    strcpy(filename_name_copy, abs_filename);

    abs_filename_path = dirname(abs_filename_path_copy);
    filename_name = basename(filename_name_copy);

    //open file
    fp = fopen(abs_filename, "r");

    //structure with informations about file -> passed as argument to nested functions (some of them need these)
    fileInfo.absName = abs_filename;
    fileInfo.absPath = abs_filename_path;
    fileInfo.name = filename_name;

    if(fp == NULL){
        fprintf(stderr, "Can't open file '%s'!\n", fileInfo.name);
        free(abs_filename);
        free(abs_filename_path_copy);
        free(filename_name_copy);
        exit(EXIT_FAILURE);
    }

    do{
        c = fgetc(fp);

        if(c == '\t') c = ' '; //simplify parsing
        if(c == '\r') continue; //compatibility with win
        if(c == '\n') lineCounter++; //counting line in file

        /*
         * this is core of the tokenizer ... it is FSM, will check
         * character and make individuals tokens (words) and find its
         * separator (space or newline)
         */

        switch(tokState){
            case IDLE:
                if(c == ';'){
                    tokState = COMMENT;
                }
                else if(c == ' ' || c == '\n'){
                    tokState = IDLE;
                }
                else{
                    tok_buff[tok_buff_top++] = (char)c;
                    tokState = TOK;
                }
                break;
            case COMMENT:
                if(c == '\n'){
                    tokState = IDLE;
                }
                else{
                    tokState = COMMENT;
                }
                break;
            case TOK:
                if(c == ' '){
                    tok_buff[tok_buff_top++] = '\0';
                    tok_buff_top = 0;
                    tokState = IDLE;

                    int x = 0;
                    while(1){
                        x = fgetc(fp);
                        if(x == '\t') x = ' ';
                        if(x == '\r') x = fgetc(fp);
                        if(x != ' ') break;
                    }

                    if(x == '\n' || x == EOF){
                        lineCounter++;
                        eval_tok(tok_buff, '\n', &fileInfo, lineCounter);
                        lineCounter--;
                    }
                    else{
                        eval_tok(tok_buff, ' ', &fileInfo, lineCounter);
                    }
                    ungetc(x, fp);
                }
                else if(c == '\n' || c == EOF){
                    tok_buff[tok_buff_top++] = '\0';
                    tok_buff_top = 0;
                    tokState = IDLE;
                    eval_tok(tok_buff, '\n', &fileInfo, lineCounter);
                }
                else{
                    tok_buff[tok_buff_top++] = (char)c;
                }
                break;
            default:
                fprintf(stderr, "Internal tokenizer error!\n");
                fclose(fp);
                free(abs_filename);
                free(abs_filename_path_copy);
                free(filename_name_copy);
                exit(EXIT_FAILURE);
        }
    }while(c != EOF);

    fclose(fp);
    free(abs_filename);
    free(abs_filename_path_copy);
    free(filename_name_copy);
}

/* -----------------------------------------------------------------------------
 * Functions for recognizing reserved keyword
 */

static const char *is_pseudo(char *s){
    /*
     * Return format string for pseudoinstruction
     *
     * P - pseudoinstruction
     * D - integer value
     * d - variable count of integer values
     * S - string
     * N - unknown (used for reporting nonpseudo instruciton)
     */

         if (strcmp(s, ".ORG")     == 0) return "PD";
    else if (strcmp(s, ".CONS")    == 0) return "PSD";
    else if (strcmp(s, ".DAT_B")   == 0) return "Pd";
    else if (strcmp(s, ".DAT_H")   == 0) return "Pd";
    else if (strcmp(s, ".DAT_W")   == 0) return "Pd";
    else if (strcmp(s, ".DS")      == 0) return "PD";
    else if (strcmp(s, ".EXPORT")  == 0) return "PS";
    else if (strcmp(s, ".IMPORT")  == 0) return "PS";
    else if (strcmp(s, ".SECTION") == 0) return "PS";
    else                                 return "N";
}

static const char* is_prepr(char* s){
    /*
     * Return format
     *
     * P - prepr cmd
     * S - string
     * s - should be string but may be also int (dont to typecheck!)
     */

         if( strcmp(s, "#define")   == 0 ) return "PS";
    else if( strcmp(s, "#ifdef")    == 0 ) return "PS";
    else if( strcmp(s, "#ifndef")   == 0 ) return "PS";
    else if( strcmp(s, "#endif")    == 0 ) return "P";
    else if( strcmp(s, "#include")  == 0 ) return "PS";
    else if( strcmp(s, "#constant") == 0 ) return "PSs";
    else                                   return "N";

}

/* -----------------------------------------------------------------------------
 * Preprocessor
 */

static void exec_prepr_cmd(char * line, fileInfo_t * fileInfo, unsigned int *false_if){
    char *linecopy = NULL;
    char *cmd[] = {NULL, NULL, NULL};
    unsigned int x = 0;

    linecopy = (char *) malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(linecopy, line);

    cmd[0] = linecopy;

    for(unsigned int i = 0; i < strlen(line); i++){
        if(linecopy[i] == ';') {
            linecopy[i] = '\0';
            cmd[++x] = &linecopy[i + 1];
        }
    }

    if( strcmp(cmd[0], "#define") == 0 ){
        if(*false_if == 0){
            save_def(cmd[1]);
        }
    }
    else if( strcmp(cmd[0], "#ifdef") == 0 ){
        if(*false_if == 0){
            if(is_def(cmd[1]) == 0){
                (*false_if)++;
            }
        }
        else{
            (*false_if)++;
        }
    }
    else if( strcmp(cmd[0], "#ifndef") == 0 ){
        if(*false_if == 0){
            if(is_def(cmd[1]) == 1){
                (*false_if)++;
            }
        }
        else{
            (*false_if)++;
        }
    }
    else if( strcmp(cmd[0], "#endif") == 0 ){
        if(*false_if > 0){
            (*false_if)--;
        }
    }
    else if( strcmp(cmd[0], "#include") == 0 ){
        char *path = NULL;
        unsigned int false_if_backup = 0;

        if(*false_if != 0) {
            free(linecopy);
            return;
        }

        path = (char *) malloc( sizeof(char) * (strlen(fileInfo->absPath) + strlen(cmd[1]) + 2) );
        strcpy(path, fileInfo->absPath);
        strcat(path, "/");
        strcat(path, cmd[1]);

        false_if_backup = *false_if;
        *false_if = 0;
        __tokenizer(path);
        *false_if = false_if_backup;
        free(path);
    }
    else if( strcmp(cmd[0], "#constant") == 0 ){
        if(*false_if == 0){
            save_cons(cmd[1], cmd[2]);
        }
    }
    else{
        fprintf(stderr, "Internal error!\n");
        exit(EXIT_FAILURE);
    }

    free(linecopy);
}

/* -----------------------------------------------------------------------------
 * Functions for evaluating word stream from tokenizer
 */

void eval_tok(char * token, char separator, fileInfo_t * fileInfo, unsigned int line){

    char *ret = is_cons(token);
    if(ret != NULL) token = ret;

    if(separator != '\n') line++;

    if(iteration == 0){

        unsigned int len = (unsigned int)strlen(token);

        if(token[0] == '.'){
            format_string = is_pseudo(token);

            if(format_string[0] == 'N'){
                fprintf(stderr, "Syntax error in file %s at: %d.\n", fileInfo->name, line);
                exit(EXIT_FAILURE);
            }
            else if(format_string[0] == 'P'){
                if(format_string[1] == '\0'){
                    eval_pseudo(token, fileInfo, line, &false_if_count);
                    actual_eval = EVAL_NONE;
                }
                else{
                    if(format_string[1] == 'd' && separator == '\n'){
                        fprintf(stderr, "Syntax error in file %s at %d.\n", fileInfo->name, line);
                        exit(EXIT_FAILURE);
                    }
                    else{
                        iteration++;
                        append_token(token);
                        actual_eval = EVAL_PSEUD;
                    }
                }
            }
            else{
                //internal error
                fprintf(stderr, "Internal error in tokenizer!\n");
                exit(EXIT_FAILURE);
            }
        }
        else if(token[0] == '#'){
            format_string = is_prepr(token);

            if(format_string[0] == 'N'){
                fprintf(stderr, "Syntax error in file %s at: %d. \n", fileInfo->name, line);
                exit(EXIT_FAILURE);
            }
            else if(format_string[0] == 'P'){
                if(format_string[1] == '\0'){
                    char t[TOKEN_EVAL_BUFF_MAX];
                    for(int i = 0; i < TOKEN_EVAL_BUFF_MAX; i++)t[i] = token[i];

                    clean_eval_buff();

                    care_about_filelist(fileInfo);
                    exec_prepr_cmd(t, fileInfo, &false_if_count);

                    actual_eval = EVAL_NONE;
                }
                else{
                    iteration++;
                    append_token(token);
                    actual_eval = EVAL_PREPR;
                }
            }
        }
        else if(token[0] == '$'){
            //macros are not supported!
            fprintf(stderr, "Macro called but they are not supported!\n");
            exit(EXIT_FAILURE);
        }
        else if(token[len - 1] == ':'){
            eval_label(token, fileInfo, line, &false_if_count);
            actual_eval = EVAL_NONE;
            //label
        }
        else{
            format_string = is_instruction(token);

            if(format_string[0] == 'N'){
                fprintf(stderr, "Syntax error in input file! in: %s@%d\n", fileInfo->name, line);
                exit(EXIT_FAILURE);
            }
            else if(format_string[0] == 'I'){
                if(format_string[1] == '\0'){
                    //instruction without argument
                    eval_instr(token, fileInfo, line, &false_if_count);
                    actual_eval = EVAL_NONE;
                }
                else{
                    iteration++;
                    append_token(token);
                    actual_eval = EVAL_INSTR;
                }
            }
            else{
                fprintf(stderr, "Internal error in tokenizer!\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    else{
        if(actual_eval == EVAL_PSEUD){

            if(format_string[iteration] == 'D'){
                if(is_numero(token)){
                    append_token(token);
                }
                else{
                    fprintf(stderr, "Syntax error in file '%s' at line %d!\n", fileInfo->name, line);
                    exit(EXIT_FAILURE);
                }
            }
            else if(format_string[iteration] == 'S'){
                if(is_string(token)){
                    append_token(token);
                }
                else{
                    fprintf(stderr, "Syntax error in file '%s' at line %d!\n", fileInfo->name, line);
                    exit(EXIT_FAILURE);
                }
            }
            else if(format_string[iteration] == 'd'){
                if(is_numero(token)){
                    append_token(token);
                    if(separator == ' ') iteration--;
                }
                else{
                    fprintf(stderr, "Syntax error in file '%s' at line %d!\n", fileInfo->name, line);
                    exit(EXIT_FAILURE);
                }
            }
            else{
                fprintf(stderr, "Unknown char in format string!\n");
                exit(EXIT_FAILURE);
            }

            if(format_string[iteration + 1] == '\0'){
                eval_pseudo(token_eval_buff, fileInfo, line, &false_if_count);
                clean_eval_buff();
                actual_eval = EVAL_NONE;
            }
            else{
                iteration++;
            }

        }
        else if(actual_eval == EVAL_LABEL){
            fprintf(stderr, "Internal error in tokenizer!\n");
            exit(EXIT_FAILURE);
        }
        else if(actual_eval == EVAL_PREPR){
            if(format_string[iteration] == 'D'){
                if(is_numero(token)){
                    append_token(token);
                }
                else{
                    fprintf(stderr, "Syntax error in file '%s' at line %d!\n", fileInfo->name, line);
                    exit(EXIT_FAILURE);
                }
            }
            else if(format_string[iteration] == 'S'){
                if(is_string(token)){
                    append_token(token);
                }
                else{
                    fprintf(stderr, "Syntax error in file '%s' at line %d!\n", fileInfo->name, line);
                    exit(EXIT_FAILURE);
                }
            }
            else if(format_string[iteration] == 's'){
                append_token(token);
            }
            else{
                fprintf(stderr, "Unknown char in format string!\n");
                exit(EXIT_FAILURE);
            }

            if(format_string[iteration + 1] == '\0'){
                char t[TOKEN_EVAL_BUFF_MAX];
                for(int i = 0; i < TOKEN_EVAL_BUFF_MAX; i++) t[i] = token_eval_buff[i];

                clean_eval_buff();

                care_about_filelist(fileInfo);
                exec_prepr_cmd(t, fileInfo, &false_if_count);

                actual_eval = EVAL_NONE;
            }
            else{
                iteration++;
            }
        }
        else if(actual_eval == EVAL_INSTR){
            if(format_string[iteration] == 'R'){
                if(is_reg(token) == 1){
                    append_token(token);
                }
                else{
                    fprintf(stderr, "Syntax error in input file! in: %s@%d\n", fileInfo->name, line);
                    exit(EXIT_FAILURE);
                }
            }
            else if(format_string[iteration] == 'c'){
                if(is_comparison(token) == 1){
                    append_token(token);
                }
                else{
                    fprintf(stderr, "Syntax error in input file! in: %s@%d\n", fileInfo->name, line);
                    exit(EXIT_FAILURE);
                }
            }
            else if(format_string[iteration] == '6' || format_string[iteration] == '4'){
                append_token(token);
            }
            else{
                fprintf(stderr, "Unknown char in format string from isa lib!\n");
                exit(EXIT_FAILURE);
            }

            if(format_string[iteration + 1] == '\0'){
                eval_instr(token_eval_buff, fileInfo, line, &false_if_count);
                clean_eval_buff();
                actual_eval = EVAL_NONE;
            }
            else{
                iteration++;
            }
        }
        else{
            fprintf(stderr, "Internal error in tokenizer!\n");
            exit(EXIT_FAILURE);
        }
    }

}

static void append_token(char * token){
    if(token_eval_buff_top != 0){
        token_eval_buff[token_eval_buff_top++] = ';';
    }

    if(token_eval_buff_top + (unsigned int)strlen(token) > TOKEN_EVAL_BUFF_MAX){
        fprintf(stderr, "Internal error! Buffer overflow!");
        exit(EXIT_FAILURE);
    }

    strcpy((char*)(token_eval_buff + token_eval_buff_top), token);
    token_eval_buff_top += (unsigned int)strlen(token);
}

static void clean_eval_buff(void){
    token_eval_buff[0] = 0;
    token_eval_buff_top = 0;
    iteration = 0;
    format_string = NULL;
}

static int is_string(char * token){
    if(!(isalpha(token[0]) || token[0] == '_')){
        return 0;
    }
    for(int i = 1; token[i] != '\0'; i++){
        if(isalnum(token[i])) continue;
        if(token[i] == '_') continue;
        if(token[i] == '.') continue;
        return 0;
    }
    return 1;
}

static int is_numero(char * token){
    for(int i = 0; token[i] != '\0'; i++){
        if(isxdigit(token[i])) continue;
        if(token[i] == 'x') continue;
        if(token[i] == 'b') continue;
        return 0;
    }
    return 1;
}

/* -----------------------------------------------------------------------------
 * Functions to handle tokenized output stream. Packaking them to output token list.
 */

static void eval_pseudo(char * token, fileInfo_t * fileInfo, unsigned int line, unsigned int *false_if){
    if((*false_if) != 0) return;

    care_about_filelist(fileInfo);

    tok_t *x = (tok_t *)malloc(sizeof(tok_t));
    pseudo_t *p = (pseudo_t *)malloc(sizeof(pseudo_t));
    char *l = (char *)malloc(sizeof(char) * ( strlen(token) + 1 ) );

    if((x == NULL) || (p == NULL) || (l == NULL)){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    strcpy(l, token);
    p->line = l;

    x->next = NULL;
    x->prev = NULL;
    x->fileInfo = get_current_fileInfoOut(fileInfo->absName);
    x->lineNumber = line;
    x->type = TOKEN_IS_PSEUD;
    x->payload.p = p;

    append_to_toklist(x);
}

static void eval_label(char * token, fileInfo_t * fileInfo, unsigned int line, unsigned int *false_if){
    if((*false_if) != 0) return;

    care_about_filelist(fileInfo);

    tok_t *x = (tok_t *)malloc(sizeof(tok_t));
    label_t *label = (label_t *)malloc(sizeof(label_t));
    char *l = (char *)malloc(sizeof(char) * ( strlen(token) + 1 ) );

    if((x == NULL) || (label == NULL) || (l == NULL)){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    strcpy(l, token);
    label->line = l;

    x->next = NULL;
    x->prev = NULL;
    x->fileInfo = get_current_fileInfoOut(fileInfo->absName);
    x->lineNumber = line;
    x->type = TOKEN_IS_LABEL;
    x->payload.l = label;

    append_to_toklist(x);
}

static void eval_instr(char * token, fileInfo_t * fileInfo, unsigned int line, unsigned int *false_if){
    if((*false_if) != 0) return;

    care_about_filelist(fileInfo);

    tok_t *x = (tok_t *)malloc(sizeof(tok_t));
    tInstruction *instr = (tInstruction *)malloc(sizeof(tInstruction));
    char *l = (char *)malloc(sizeof(char) * ( strlen(token) + 1 ) );

    if((x == NULL) || (instr == NULL) || (l == NULL)){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    strcpy(l, token);
    instr->line = l;

    x->next = NULL;
    x->prev = NULL;
    x->fileInfo = get_current_fileInfoOut(fileInfo->absName);
    x->lineNumber = line;
    x->type = TOKEN_IS_INSTR;
    x->payload.i = instr;

    append_to_toklist(x);
}

static void append_to_toklist(tok_t *x){
    if(toklist_first == NULL){
        toklist_first = x;
        toklist_last = x;
    }
    else{
        toklist_last->next = x;
        x->prev = toklist_last;
        toklist_last = x;
    }
}

/* -----------------------------------------------------------------------------
 * Functions for handling constants and defines
 */

static void save_def(char * s){
    def_t * newconst = malloc(sizeof(def_t));
    char * ns = malloc(sizeof(char)*(strlen(s) + 1));

    if(newconst == NULL || ns == NULL){
        fprintf(stderr, "malloc failed!");
        exit(EXIT_FAILURE);
    }

    strcpy(ns, s);

    newconst->constant = ns;
    newconst->next = NULL;

    if(def_list == NULL){
        def_list = newconst;
    }
    else{
        def_t* last_item = NULL;
        for(last_item = def_list; last_item->next != NULL; last_item = last_item->next);
        last_item->next = newconst;
    }
}

static int is_def(char *s){
    if(def_list == NULL) return 0;
    for(def_t* item = def_list; item != NULL; item = item->next){
        if(strcmp(item->constant, s) == 0) return 1;
    }
    return 0;
}

static void save_cons(char * s, char * v){
    cons_t* newconst = malloc(sizeof(cons_t));
    char* ns = malloc(sizeof(char)*(strlen(s) + 1));
    char* nv = malloc(sizeof(char)*(strlen(v) + 1));

    if(newconst == NULL || ns == NULL || nv == NULL){
        fprintf(stderr, "malloc failed!");
        exit(EXIT_FAILURE);
    }

    strcpy(ns, s);
    strcpy(nv, v);

    newconst->constant = ns;
    newconst->value = nv;
    newconst->next = NULL;

    if(cons_list == NULL){
        cons_list = newconst;
    }
    else{
        cons_t* last_item = NULL;
        for(last_item = cons_list; last_item->next != NULL; last_item = last_item->next);
        last_item->next = newconst;
    }
}

static char* is_cons(char * s){
    if(cons_list == NULL) return NULL;
    for(cons_t* item = cons_list; item != NULL; item = item->next){
        if(strcmp(item->constant, s) == 0) return item->value;
    }
    return NULL;
}

/* -----------------------------------------------------------------------------
 * Functions to handle list of files.
 */

static void care_about_filelist(fileInfo_t *fileInfo){

    if(filelist_last != NULL){
        if( strcmp(filelist_last->absName, fileInfo->absName) != 0 ){  //check if we are in same file as last
            //we chaged file
            if(is_in_list(fileInfo->absName) == 0){   //check if this file is already in list
                //create new filelist item
                fileInfoOut_t *newInfo = make_new_fileitem(fileInfo);

                //connect new item into  list
                filelist_last->next = newInfo;
                newInfo->prev = filelist_last;
                filelist_last = newInfo;
            }
            else{   //already in the list
                return;
            }
        }
        else{ //running in the same file
            return;
        }
    }
    else{
        //list isn't exist ... that mean this is first run

        //anyway we need new item
        fileInfoOut_t *newInfo = make_new_fileitem(fileInfo);
        filelist_first = newInfo;
        filelist_last = newInfo;
    }
}

static inline fileInfoOut_t * make_new_fileitem(fileInfo_t *fileInfo){
    char *x1 = (char *)malloc( sizeof(char) * (strlen(fileInfo->absName) + 1) );
    char *x2 = (char *)malloc( sizeof(char) * (strlen(fileInfo->absPath) + 1) );
    char *x3 = (char *)malloc( sizeof(char) * (strlen(fileInfo->name) + 1) );
    fileInfoOut_t *newInfo = (fileInfoOut_t *)malloc( sizeof(fileInfoOut_t) );

    //check malloc result
    if((x1 == NULL) || (x2 == NULL) || (newInfo == NULL)){
        fprintf(stderr, "Malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    //copy path
    strcpy(x1, fileInfo->absName);
    strcpy(x2, fileInfo->absPath);
    strcpy(x3, fileInfo->name);

    //insert content of the new item
    newInfo->absName = x1;
    newInfo->absPath = x2;
    newInfo->name = x3;
    newInfo->prev = NULL;
    newInfo->next = NULL;

    return newInfo;
}

static int is_in_list(char *absName){
    for(fileInfoOut_t *t = filelist_first; t != NULL; t = t->next){
        if(strcmp(absName, t->absName) == 0){
            return 1;
        }
    }
    return 0;
}

static fileInfoOut_t * get_current_fileInfoOut(char *fileNameAbs){
    for(fileInfoOut_t *t = filelist_first; t != NULL; t = t->next){
        if(strcmp(fileNameAbs, t->absName) == 0){
            return t;
        }
    }
    return NULL;
}

/* -----------------------------------------------------------------------------
 * Clean up functions.
 */

void tokenizer_cleanup(void){

    {   //freeFileInfoOut list
        fileInfoOut_t *tmp = NULL;
        fileInfoOut_t *head = filelist_first;

        while(head != NULL){
            tmp = head;
            head = head->next;

            free(tmp->absName);
            free(tmp->absPath);
            free(tmp->name);
            free(tmp);
        }
    }

    {   //free tokens list
        tok_t *tmp = NULL;
        tok_t *head = toklist_first;

        while(head != NULL){
            tmp = head;
            head = head->next;

            if(tmp->type == FLAGS_IS_INSTR){
                cleanup_istruction_struct(tmp->payload.i);
                free(tmp->payload.i);
            }
            else if(tmp->type == FLAGS_IS_LABEL){
                free(tmp->payload.l->line);
                free(tmp->payload.l);
            }
            else if(tmp->type == FLAGS_IS_PSEUD){
                free(tmp->payload.p->line);
                free(tmp->payload.p);
            }
            else{
                break;
            }
            free(tmp);
        }
    }

    {   //free defs list
        def_t *tmp = NULL;
        def_t *head = def_list;

        while(head != NULL){
            tmp = head;
            head = head->next;

            free(tmp->constant);
            free(tmp);
        }
    }

    {   //free constant list
        cons_t *tmp = NULL;
        cons_t *head = cons_list;

        while(head != NULL){
            tmp = head;
            head = head->next;

            free(tmp->value);
            free(tmp->constant);
            free(tmp);
        }
    }

}

/* -----------------------------------------------------------------------------
 * Debug functions
 */

#ifdef DEBUG

void print_filelist(void){
    printf("\nFile info list: \n");
    if(filelist_first == NULL){
        printf("  - List is empty\n");
    }
    else{
        for(fileInfoOut_t *t = filelist_first; t != NULL; t = t->next){
            printf("  - '%s'\n", t->absName);
        }
    }
}

void print_defs(void){
    printf("\nDefines:\n");
    if(def_list == NULL){
        printf("  - List is empty\n");
    }
    else{
        for(def_t* item = def_list; item != NULL; item = item->next){
            printf("  - '%s'\n", item->constant);
        }
    }
}

void print_cons(void){
    printf("\nConstants:\n");
    if(cons_list == NULL){
        printf("  - List is empty\n");
    }
    else{
        for(cons_t* item = cons_list; item != NULL; item = item->next){
            printf("  - '%s' : '%s'\n", item->constant, item->value);
        }
    }
}

void print_toklist(void){
    printf("\nOutput token list: \n");
    if(toklist_first == NULL){
        printf("  - List is empty\n");
    }
    else{
        for(tok_t *t = toklist_first; t != NULL; t = t->next){
            if(t->type == TOKEN_IS_INSTR){
                printf("  - from %s @ %d \t Instr: '%s'\n", t->fileInfo->name, t->lineNumber, t->payload.i->line);
            }
            else if(t->type == TOKEN_IS_LABEL){
                printf("  - from %s @ %d \t Label: '%s'\n", t->fileInfo->name, t->lineNumber, t->payload.l->line);
            }
            else if(t->type == TOKEN_IS_PSEUD){
                printf("  - from %s @ %d \t Pseud: '%s'\n", t->fileInfo->name, t->lineNumber, t->payload.p->line);
            }
            else{
                fprintf(stderr, "Internal error in tokenizer!\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#endif
