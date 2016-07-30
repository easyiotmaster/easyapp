#include <stdio.h>
#include <string.h>

#include "atparse.h"

#define ATCMD_NAME_LENGTH 16

int parse_at_cmd_string(char *at , ATCMD_DATA *ad)
{
    int atstrlen = strlen(at)+1;//只有加1才能识别最后一个字符串 0x0
    int i=0;
    char parse_status = 0;
    char appendname_index = 0; //写NAME的索引

    //ad->name[0] = 0x0; //清空name字符串
    memset(ad,0x0,sizeof(ATCMD_DATA));

    for(i=0;i<atstrlen;i++)
    {
        switch(parse_status)
        {
        case PARSER_STATUS_INIT:
        {
            if ((i==0) && (at[i]=='A'))
            {
                parse_status = PARSER_STATUS_RECV_A;
            }
            else if((i==0) && (at[i]=='+'))
            {
                parse_status = PARSER_STATUS_RECV_PLUS;
            }
            else{
                goto parse_err;
            }
            break;
        }

        case PARSER_STATUS_RECV_A:
        {
            if ((i==1) && (at[i]=='T'))
            {
                parse_status = PARSER_STATUS_RECV_T;
            }else{
                goto parse_err;
            }
            break;
        }
        case PARSER_STATUS_RECV_T:
        {
            if ((i==2) && (at[i]=='+'))
            {
                parse_status = PARSER_STATUS_RECV_PLUS;
            }else{
                goto parse_err;
            }
            break;
        }
        case PARSER_STATUS_RECV_PLUS:
        {
            //已经收到 + 号了，所以可以开始进行NAME解析了，但是必须要判断是否在‘A’~‘Z’之间
            if ((at[i]>='A') && (at[i]<='Z'))
            {
                ad->name[(int)appendname_index++] = at[i];

                //如果名字长度超长，则认为越界了
                if (appendname_index >= (ATCMD_NAME_LENGTH-1))
                {
                    goto parse_err;
                }
                //
            }else if (at[i] == '='){
                parse_status = PARSER_STATUS_STARTPARSE;
            }else if ((at[i] == 0x0d) || (at[i] == 0x00)){
                parse_status = PARSER_STATUS_PARSE_END;
            }else{
                goto parse_err;
            }
            break;
        }

        case PARSER_STATUS_STARTPARSE: //开始解析参数
        {
            //param_cnt ++; //说明已经有一个参数了，应该 +1

            //下面开始解析参数
            if(at[i] == '\"') //如果参数的第一个是 引号，则认为是字符串类型的参数
            {
                ad->param[ad->param_count].type = ATCMD_PARAM_STRING_TYPE;
                ad->param[ad->param_count].pos = &at[i]; //从 引号 开始的位置
                ad->param_count ++;
                parse_status = PARSER_STATUS_PARSE_STRING;
            }else if ((at[i]>='0') && (at[i]<='9'))
            {
                ad->param[ad->param_count].type = ATCMD_PARAM_INTEGER_TYPE;
                ad->param[ad->param_count].pos = &at[i]; //从 引号 开始的位置
                ad->param_count ++;
                parse_status = PARSER_STATUS_PARSE_INT;
            }
            else{
                goto parse_err;
            }

            break;
        }

        case PARSER_STATUS_PARSE_STRING:
        {
            //只有再一次读到引号才认为是末尾了
            if (at[i] == '\"')
            {
                parse_status = PARSER_STATUS_PARSE_STRING_END;
            }
            break;
        }
        case PARSER_STATUS_PARSE_STRING_END: //解析字符串结束
        {
            if (at[i] == ',')
            {
                //继续解析下一个参数
                parse_status = PARSER_STATUS_STARTPARSE;
            }else if ((at[i] == 0x0d) || (at[i] == 0x00)){
                //代表解析结束
                parse_status = PARSER_STATUS_PARSE_END;
								return 0;
            }else{
                //如果是其他的就认为是错误的
                goto parse_err;
            }

            break;
        }
        case PARSER_STATUS_PARSE_INT:
        {
            if (at[i] == ',')
            {
                //继续解析下一个参数
                parse_status = PARSER_STATUS_STARTPARSE;
            }else if ((at[i] == 0x0d) || (at[i] == 0x00)){
                //代表解析结束
                parse_status = PARSER_STATUS_PARSE_END;
								return 0;
            }else{
                //如果解析数字参数过程中出现其他字符，则认为解析失败
                if ((at[i]>='0') && (at[i]<='9'))
                {}else{
                    goto parse_err;
                }
            }
            break;
        }

        case PARSER_STATUS_PARSE_END:
        {
            return 0;    //解析结束就直接返回 0
            break;
        }


        default:
        {
            break;
        }
        }
    }


parse_err:
    return -1;

}

int get_at_param_int(ATCMD_DATA *ad, char idx)
{
    int ret;
    char numstr[8];
    char i=0;
    char *str = ad->param[(int)idx].pos;
    //int strl = strlen(str);
    memset(numstr,8,0x0);
    for(i=0;i<7;i++)
    {
        if ((str[(int)i]<'0') || (str[(int)i]>'9'))
            break;
        numstr[(int)i] = str[(int)i];
    }

    sscanf(numstr,"%d",&ret);

    return ret;


}


char *get_at_param_string(ATCMD_DATA *ad, char idx , char *outstrbuf , int outstrbuf_length)
{
    short i=0;
    short j=0;
    char *str = ad->param[(int)idx].pos;
    int strl = strlen(str);

#if 1
    if (str[0] != '\"')
    {
        outstrbuf[0] = 0x0;
        return 0;
    }
#endif

    for(i=1;i<strl;i++)
    {
        if (str[i] == '\"')
            break;
        outstrbuf[j++] = str[i];
    }

    //添加字符串末尾
    outstrbuf[j] = 0x0;

    return outstrbuf;
}

#define printf(...)
int test(int argc , char **argv)
{
    int i=0;
    int ret;
    ATCMD_DATA ad;
    char outstring[32];
    //char test_atcmd[] = "AT+FUCK\r\n";
    char test_atcmd[] = "AT+FUCK=13123,123123,12,3,123,\"555\",3123\r\n";
    //int parse_at_cmd_string(char *at , struct ATCMD_DATA *ad)
    //char test_atcmd[] = "AT+FUCK=13123,123123,12,3\"\r\n";
    ret = parse_at_cmd_string(test_atcmd,&ad);
    if (ret == 0)
    {
        printf("NAME : %s\r\n",ad.name);
        for(i=0;i<ad.param_count;i++)
        {
            switch(ad.param[i].type)
            {
            case ATCMD_PARAM_STRING_TYPE:
                printf("string : %s\r\n",get_at_param_string(&ad,i,outstring,sizeof(outstring)));
                break;
            case ATCMD_PARAM_INTEGER_TYPE:
                printf("INT : %d \r\n",get_at_param_int(&ad,i));
                break;

            }
            printf("XXXXXXXXX:[%d] %s\r\n",i,ad.param[i].pos);
        }
    }else{
        printf("error\r\n");
    }

    return 0;
}

/*int parse_at_cmd_string(char *at , struct ATCMD_DATA *ad);
char *get_at_param_string(struct ATCMD_DATA *ad, char idx , char *outstrbuf , int outstrbuf_length);
int get_at_param_int(struct ATCMD_DATA *ad, char idx);*/

