/*! \mainpage parse.c DECT解析器
 *	\section About 关于
 *  
 *  \n 版本: parse
 *  \n 用途: 解析DECT协议
 *  \n 语言: C (ANSI)
 *  \n 平台: LINUX
 *  \n 编译: Gcc
 *  \n 作者: 刘元龙
 *
 *  \section Log 日志
 *  \n 2017 v0.1 初版
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>

/************************************************************************/
/*                                                                      */
/************************************************************************/
typedef struct
{
    char ifile[128];
    char ofile[128];
    int  len; 
}ST_ARG;

ST_ARG g_arg = {0,};

/*
#define  BM02_ANT_SEL_BIT             0
#define  BM02_FAD_BIT                 1
#define  BM02_WIDE_BIT                2
#define  BM02_NO_SYNC_BIT             3
#define  BM02_T_DELAY_BIT             4
#define  BM02_TIME_BASE_BIT           5
#define  BM02_FORCE_SYNC_BIT          6
#define  BM02_FP_PP_MODE_BIT          7
#define  BM02_SIF_LEN_SEL_BIT         8
#define  BM02_SUBFIELD_TYPE_BIT      14
#define  BM02_LONG_PREAMBLE_BIT      16
#define  BM02_SCT_COND_SEL_BIT       16
#define  BM02_TX_BIT                 17
#define  BM02_PROTECTED_BIT          18
#define  BM02_PACKET_TYPE_BIT        19
#define  BM02_CHECK_ACCESS_BIT       21
#define  BM02_CIPHER_BIT             22
#define  BM02_NO_SCRAMBLE_BIT        23
#define  BM02_B_MUTE_BIT             24
#define  BM02_SLC_CTRL_SEL_BIT       25
#define  BM02_B_BYTE_ARRAY_BIT       27
#define  BM02_ULE_NODE_MODE_BIT        28
#define  BM02_SCT_COND_INT_BIT       29
#define  BM02_SCT_SLOT_INT_BIT       30
#define  BM02_SCT_ACTIVE_BIT         31
*/
typedef union
{
    unsigned int ctrl;
    struct
    {
        unsigned int BM02_ANT_SEL_BIT:1;
        unsigned int BM02_FAD_BIT:1;
        unsigned int BM02_WIDE_BIT:1;
        unsigned int BM02_NO_SYNC_BIT:1;
        unsigned int BM02_T_DELAY_BIT:1;
        unsigned int BM02_TIME_BASE_BIT:1;
        unsigned int BM02_FORCE_SYNC_BIT:1;
        unsigned int BM02_FP_PP_MODE_BIT:1;
        unsigned int BM02_SIF_LEN_SEL_BIT:6;
        unsigned int BM02_SUBFIELD_TYPE_BIT:2;
        unsigned int BM02_LONG_PREAMBLE_or_SCT_CON_SEL_BIT:1;
        unsigned int BM02_TX_BIT:1;
        unsigned int BM02_PROTECTED_BIT:1;
        unsigned int BM02_PACKET_TYPE_BIT:2;
        unsigned int BM02_CHECK_ACCESS_BIT:1;
        unsigned int BM02_CIPHER_BIT:1;
        unsigned int BM02_NO_SCRAMBLE_BIT:1;
        unsigned int BM02_B_MUTE_BIT:1;
        unsigned int BM02_SLC_CTRL_SEL_BIT:2;
        unsigned int BM02_B_BYTE_ARRAY_BIT:1;
        unsigned int BM02_ULE_NODE_MODE_BIT:1;
        unsigned int BM02_SCT_COND_INT_BIT:1;
        unsigned int BM02_SCT_SLOT_INT_BIT:1;
        unsigned int BM02_SCT_ACTIVE_BIT:1;
    };
}ST_BMP;

ST_BMP g_bmp = {0,};

const char *sBmpCtrlPacketType[]={
    "full",
    "double",
    "short",
    "long"
};
/************************************************************************/
/*                                                                      */
/************************************************************************/
static void display_help (void)
{
    printf("\n"
        "[" __DATE__ " " __TIME__ "]\n"
        "        --help            display this help and exit\n"
        "        --version         output version information and exit\n"
        "-i      --input           input file path\n"
        "-o      --output          output file path\n"
        "-l      --length          max length to output\n"
        "\n"
        );
    exit(0);
}

static int parse_options(int argc, const char *argv[])
{

    for (;;) 
    {
        int option_index = 0;
        static const char  *short_options = "cade:i:o:l:";
        static const struct option long_options[] = 
        {
            /*server common define*/
            {"help",            no_argument,        0, 0},
            {"version",         no_argument,        0, 0},
            {"console",         no_argument,        0, 'c'},
            {"anonymous",       no_argument,        0, 'a'},
            {"deamon",          no_argument,        0, 'd'},
            {"execute",         required_argument,  0, 'e'},

            /*application define*/
            {"input",           no_argument,        0, 'i'},
            {"output",          no_argument,        0, 'o'},
            {"length",          no_argument,        0, 'l'},
            {0, 0, 0, 0},
        };

        int c = getopt_long(argc, (char **)argv, short_options,long_options, &option_index);
        if (c == EOF) 
        {
            break;
        }

        switch (c) 
        {
        case 0:
            switch (option_index)
            {
            case 0:
                display_help();
                break;
            case 1:
                //display_version();
                break;
            }
            break;
            
        case 'i':
            {
                strncpy(g_arg.ifile, optarg, 128);
                break;
            }
            break;
            
        case 'o':
            {
                strncpy(g_arg.ofile, optarg, 128);
                break;
            }
            break;
            
        case 'l':
            {                
                if(sscanf(optarg,"0x%x",&g_arg.len)!=1)
                {
                    display_help();
                }
            }
            break;
            
        case '?':
        default:
            display_help();
            break;
        }
    }

    return 0;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
unsigned long get_file_length(FILE* file_ptr)  
{  
    unsigned long PosCur   = 0;  
    unsigned long PosBegin = 0;  
    unsigned long PosEnd   = 0;  
    
    if(NULL == file_ptr)  
    {  
        return   0;  
    }
    
    PosCur = ftell(file_ptr);  //得到文件指针的当前位置
    fseek(file_ptr, 0L, SEEK_SET);  //将文件指针移到文件开始
    PosBegin = ftell(file_ptr);  //得到当前文件的位置，也就是文件头的位置
    fseek(file_ptr, 0L, SEEK_END);  //将文件指针移到文件末尾
    PosEnd = ftell(file_ptr);  //得到文件的末尾位置
    fseek(file_ptr, PosCur, SEEK_SET);  //将指针移到文件的原来位置
    
    return PosEnd - PosBegin;        //返回文件大小
}

void showBinaryEx(unsigned int n, char maxbits, char showMinBit, char showMaxBit)
{   
    if(n || maxbits)
        showBinaryEx(n/2, maxbits?maxbits-1:0, showMinBit, showMaxBit);
    else   
        return;
    
    if((maxbits > showMaxBit) || (maxbits < showMinBit))
        printf(".");
    else if(showMinBit || showMaxBit)
        printf("%d",n%2);
}

/*
*  n: num to show as binary
*  maxbits: max bits to be print
*  showMinBit: 0-maxbits-1
*  showMaxBit: 0-maxbits-1
*/
void showBinary(unsigned int n, char maxbits, char showMinBit, char showMaxBit, const char *fmt, ...)
{
    if(maxbits < showMinBit || maxbits < showMaxBit)
    {
        printf("%s error!\n",__FUNCTION__);
        return;
    }
    
    showBinaryEx(n, maxbits, maxbits-showMaxBit, maxbits-showMinBit);
    if(fmt)
    {
        static char tmpbuf[256];
        va_list args;
        int     len = 0;
        
        va_start(args, fmt);
        len += vsnprintf(tmpbuf+len,sizeof(tmpbuf)-len-1,fmt, args);
        va_end(args);
        printf("  %s\n", tmpbuf);
    }
}

void showBinaryPos(unsigned long data, int alignedBits, char fromBit, char toBit, const char *fmt, ...)
{
    char firstDots, lastDots, i;
    
    if(fromBit > toBit || alignedBits <= 0)
    {
        printf("%s error!\n",__FUNCTION__);
        return;
    }
    
    /* show first dots */
    firstDots = fromBit % alignedBits;
    for(i=0; i<firstDots; i++)
    {
        printf(".");
    }
    
    /* show data */
    for(i=fromBit; i<=toBit; i++)
    {
        printf("%d",data&(1<<(toBit-i))?1:0);
    }
    
    /* show last dots */
    if((toBit-fromBit < alignedBits) && ((toBit % alignedBits)+1 < alignedBits))
    {
        lastDots = alignedBits - (toBit % alignedBits) - 1;
        for(i=0; i<lastDots; i++)
        {
            printf(".");
        }
    }
    
    /* show comment */
    if(fmt)
    {
        static char tmpbuf[512];
        va_list args;
        int     len = 0;
        
        va_start(args, fmt);
        len += vsnprintf(tmpbuf+len,sizeof(tmpbuf)-len-1,fmt, args);
        va_end(args);
        printf("  %s\n", tmpbuf);
    }
}

/*****************************************************************************************
* @brief  : get value from offset a to offset b.
* @param  : AField: AField data
            a     : start bit offset, A0-A47, bit offset as ETSI used.
            b     : end bit offset, A0-A47, bit offset as ETSI used
* @warning: long may be 4/8 bytes
* @return : -1=fail, others=value of a->b.
******************************************************************************************/
long CodeGetBitsValue(unsigned char *code, unsigned char a, unsigned char b)
{
    char len = b-a+1;
    char i, offsetByte = 0, offsetBit = 0;
    long curBit = 0;
    long val = 0;
    
    if( !code || (a > b) || (a > 31) || (b > 31) || (len > (sizeof(unsigned long)<<3)) )
    {
        printf("GetBits Error!");
        return -1;
    }
    
    for(i=a; i<=b; i++)
    {
        offsetByte = i/8;
        offsetBit = 7-(i%8);
        curBit = (code[offsetByte] >> offsetBit) & 1;
        val |= curBit<<(b-i);
    }
    
    return val;
}

long CtrlData(unsigned char a, unsigned char b)
{
    char len = b-a+1;
    char i, offsetByte = 0, offsetBit = 0;
    long curBit = 0;
    long val = 0;
    unsigned char *ctrl = (unsigned char *)&g_bmp.ctrl;
    
    if( (a > b) || (a > 31) || (b > 31) || (len > (sizeof(unsigned long)<<3)) )
    {
        printf("GetBits Error!");
        return -1;
    }
    
    for(i=a; i<=b; i++)
    {
        offsetByte = i>>3; //i/8;
        offsetBit = 7-(i&7); //7-(i%8);
        curBit = (ctrl[offsetByte] >> offsetBit) & 1;
        val |= curBit<<(b-i);
    }
    
    return val;
}

/*****************************************************************************************
* @brief  : show value & comment from offset a to offset b.
* @param  : a     : start bit offset, A0-A47, bit offset as ETSI used.
            b     : end bit offset, A0-A47, bit offset as ETSI used
            fmt   : string to show as comment
* @warning: 
* @return : 
******************************************************************************************/
void CtrlShow(unsigned char a, unsigned char b, const char *fmt, ...)
{
    static char tmpbuf[512];
    va_list args;
    int     len = 0;
    unsigned char *ctrl = (unsigned char *)&g_bmp.ctrl;
    
    va_start(args, fmt);
    len += vsnprintf(tmpbuf+len,sizeof(tmpbuf)-len-1,fmt, args);
    va_end(args);
    showBinaryPos(CodeGetBitsValue(ctrl, a, b), 8, a, b, tmpbuf);
}

int parseBmp(unsigned int ctrl)
{
    printf("---------------------------------------\n");
    printf("#Ctrl: %08X\n", ctrl);
    
    /* Store afield to use */
    g_bmp.ctrl = ctrl;

    printf("BMP.ctrl.BM02_ANT_SEL_BIT = %d\n", g_bmp.BM02_ANT_SEL_BIT);
    printf("BMP.ctrl.BM02_FAD_BIT = %d\n", g_bmp.BM02_FAD_BIT);
    printf("BMP.ctrl.BM02_WIDE_BIT = %d\n", g_bmp.BM02_WIDE_BIT);
    printf("BMP.ctrl.BM02_NO_SYNC_BIT = %d\n", g_bmp.BM02_NO_SYNC_BIT);
    printf("BMP.ctrl.BM02_T_DELAY_BIT = %d\n", g_bmp.BM02_T_DELAY_BIT);
    printf("BMP.ctrl.BM02_TIME_BASE_BIT = %d\n", g_bmp.BM02_TIME_BASE_BIT);
    printf("BMP.ctrl.BM02_FORCE_SYNC_BIT = %d\n", g_bmp.BM02_FORCE_SYNC_BIT);
    printf("BMP.ctrl.BM02_FP_PP_MODE_BIT = %d\n", g_bmp.BM02_FP_PP_MODE_BIT);
    printf("BMP.ctrl.BM02_SIF_LEN_SEL_BIT = %d\n", g_bmp.BM02_SIF_LEN_SEL_BIT);
    printf("BMP.ctrl.BM02_SUBFIELD_TYPE_BIT = %d\n", g_bmp.BM02_SUBFIELD_TYPE_BIT);
    printf("BMP.ctrl.BM02_LONG_PREAMBLE_BIT/BM02_SCT_CON_SEL_BIT = %d\n", g_bmp.BM02_LONG_PREAMBLE_or_SCT_CON_SEL_BIT);
    printf("BMP.ctrl.BM02_TX_BIT = %d\n", g_bmp.BM02_TX_BIT);
    printf("BMP.ctrl.BM02_PROTECTED_BIT = %d\n", g_bmp.BM02_PROTECTED_BIT);
    printf("BMP.ctrl.BM02_PACKET_TYPE_BIT = %d (%s)\n", g_bmp.BM02_PACKET_TYPE_BIT, sBmpCtrlPacketType[g_bmp.BM02_PACKET_TYPE_BIT]);
    printf("BMP.ctrl.BM02_CHECK_ACCESS_BIT = %d\n", g_bmp.BM02_CHECK_ACCESS_BIT);
    printf("BMP.ctrl.BM02_CIPHER_BIT = %d\n", g_bmp.BM02_CIPHER_BIT);
    printf("BMP.ctrl.BM02_NO_SCRAMBLE_BIT = %d\n", g_bmp.BM02_NO_SCRAMBLE_BIT);
    printf("BMP.ctrl.BM02_B_MUTE_BIT = %d\n", g_bmp.BM02_B_MUTE_BIT);
    printf("BMP.ctrl.BM02_SLC_CTRL_SEL_BIT = %d\n", g_bmp.BM02_SLC_CTRL_SEL_BIT);
    printf("BMP.ctrl.BM02_B_BYTE_ARRAY_BIT = %d\n", g_bmp.BM02_B_BYTE_ARRAY_BIT);
    printf("BMP.ctrl.BM02_ULE_NODE_MODE_BIT = %d\n", g_bmp.BM02_ULE_NODE_MODE_BIT);
    printf("BMP.ctrl.BM02_SCT_COND_INT_BIT = %d\n", g_bmp.BM02_SCT_COND_INT_BIT);
    printf("BMP.ctrl.BM02_SCT_SLOT_INT_BIT = %d\n", g_bmp.BM02_SCT_SLOT_INT_BIT);
    printf("BMP.ctrl.BM02_SCT_ACTIVE_BIT = %d\n", g_bmp.BM02_SCT_ACTIVE_BIT);
    printf("\n\n");
    
    return 0;
}

void stdinParse(void)
{
    unsigned int bmpctrl;
    char c;
    int ret;
    /* use Ctrl+C to break */
    while(1)
    {
        printf("Please input bmp ctrl value(1 DWORD):\n");
        printf("> ");
        if(1 != (ret= scanf("%08X", &bmpctrl)))
        {
            printf("INPUT ERROR! r=%d\n",ret);
            while((c=getchar())!='\n'&&c!=EOF);
            continue;
        }
        if(feof(stdin)||ferror(stdin))
        {
            //如果用户输入文件结束标志（或文件已被读完），或者发生读写错误，则退出循环
            //dosomething
            break;
        }
        //没有发生错误，清空输入流。通过while循环把输入流中的余留数据“吃”掉
        while((c=getchar())!='\n'&&c!=EOF);
        //可直接将这句代码当成fflush(stdin)的替代，直接运行可清除输入缓存流
        //使用scanf("%*[^\n]");也可以清空输入流，不过会残留\n字符。
        
        /*解析BMP.ctrl*/
        parseBmp(bmpctrl);
    }
}

int main(int argc, char *argv[])
{
    FILE *inFile = NULL;
    unsigned int ctrl;
    char tmpstr[1024];

    /* parse options */
    parse_options(argc, (const char **)argv);
    
    /* input from stdin */
    if(g_arg.ifile[0] == 0)
    {
        stdinParse();
        return 0;
    }
    
    /* open file */
    if((inFile=fopen(g_arg.ifile,"rb"))==NULL)
    {
        printf("Input file error: %s\n",g_arg.ifile);
        display_help();
        return -1;
    }
    
    /*将指针设置至文件开头*/
    fseek(inFile,0,SEEK_SET);
    while(!feof(inFile))
    {
        /*读取AField*/
        fscanf(inFile,"%08X", &ctrl);
        /*读到本行结束*/
        fgets(tmpstr, 1024 , inFile);
        /*解析AField*/
        parseBmp(ctrl);
    }
    fclose(inFile);    
    return 0;
}