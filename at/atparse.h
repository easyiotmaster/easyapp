#ifndef __atparse_h__
#define __atparse_h__
#define ATCMD_NAME_LENGTH 16

enum {
    ATCMD_PARAM_STRING_TYPE,
    ATCMD_PARAM_INTEGER_TYPE,
};

struct ATCMD_PARAM {
    char type;
    char *pos;
};
struct ATCMD_DATA {
    char *atstr;
    char name[ATCMD_NAME_LENGTH];
    int param_count;
    struct ATCMD_PARAM param[16];
};

enum {
    PARSER_STATUS_INIT,
    PARSER_STATUS_RECV_A,
    PARSER_STATUS_RECV_T,
    PARSER_STATUS_RECV_PLUS,
    PARSER_STATUS_STARTPARSE,//PARSER_STATUS_RECV_EQUAL,
    PARSER_STATUS_PARSE_STRING,
    PARSER_STATUS_PARSE_STRING_END,
    PARSER_STATUS_PARSE_INT,
    PARSER_STATUS_PARSE_END,

};

int parse_at_cmd_string(char *at , ATCMD_DATA *ad);

char *get_at_param_string(ATCMD_DATA *ad, char idx , char *outstrbuf , int outstrbuf_length);
int get_at_param_int(ATCMD_DATA *ad, char idx);


#endif
