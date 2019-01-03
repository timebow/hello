/*! \mainpage main.c DECT-BMP-SCT解析器
 *	\section About 关于
 *  
 *  \n 版本: parse
 *  \n 用途: 解析SCT控制数据
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

#define DECT_BMP_CTRL_TYPE_SPT 0
#define DECT_BMP_CTRL_TYPE_SCT 1
typedef struct
{
    unsigned char  type; //0=spt, 1=sct
    unsigned short spt;
    unsigned int   sct;    
}ST_DECT;

ST_DECT g_dect = {0};

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
        "-v      --device          which device own this message(fp/pp)\n"
        "-r      --direction       which direction this message produce(tx/rx)\n"
        "\n"
        );
    exit(0);
}

static int parse_options(int argc, const char *argv[])
{

    for (;;) 
    {
        int option_index = 0;
        static const char  *short_options = "cade:i:o:l:v:r:";
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
            {"device",          no_argument,        0, 'v'},
            {"direction",       no_argument,        0, 'r'},
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

            }
            break;
            
        case 'v':
            {

            }
            break;
            
        case 'r':
            {

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

/*
*  data      : data to show as binary, u64 is max to be showed
*  maxbits   : max bits to be print
*  showMinBit: 0-maxbits-1
*  showMaxBit: 0-maxbits-1
*/
void showBinaryPos(unsigned long long data, int alignedBits, char fromBit, char toBit, const char *fmt, ...)
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
* @param  : data    : data ptr
            maxbits : maxbits
            a       : start bit offset, A0-Ax (x = maxbytes*8 - 1)
            b       : end bit offset, A0-Ax (x = maxbytes*8 - 1)
* @warning: long long, max 8 bytes
* @return : -1=fail, others=value of a->b.
******************************************************************************************/
unsigned long long GetBitsValue(unsigned char *data, unsigned int maxbytes, unsigned char a, unsigned char b)
{
    unsigned int maxbits = maxbytes << 3;
    char len = b-a+1;
    char i, offsetByte = 0, offsetBit = 0;
    long curBit = 0;
    long val = 0;
    
    if( !data || (a > b) || (a >= maxbits) || (b >= maxbits) || (len > (sizeof(unsigned long long)<<3)) )
    {
        printf("GetBits Error!");
        return -1;
    }
    
    for(i=a; i<=b; i++)
    {
        offsetByte = i/8;
        offsetBit = 7-(i%8);
        curBit = (data[offsetByte] >> offsetBit) & 1;
        val |= curBit<<(b-i);
    }
    
    return val;
}

void Show(unsigned char *data, unsigned int maxbytes, unsigned char a, unsigned char b, const char *fmt, ...)
{
    static char tmpbuf[512];
    va_list args;
    
    va_start(args, fmt);
    vsnprintf(tmpbuf,sizeof(tmpbuf)-1,fmt, args);
    va_end(args);
    showBinaryPos(GetBitsValue(data, maxbytes, a, b), 8, a, b, tmpbuf);
}

/*****************************************************************************************
* @brief  : show value & comment from offset a to offset b.
* @param  : a     : start bit offset, A0-A47, bit offset as ETSI used.
            b     : end bit offset, A0-A47, bit offset as ETSI used
            fmt   : string to show as comment
* @warning: 
* @return : 
******************************************************************************************/
unsigned long long GetSctValue(unsigned char a, unsigned char b)
{
    return GetBitsValue((unsigned char *)&g_dect.sct, sizeof(g_dect.sct), a, b);
}

void ShowSct(unsigned char a, unsigned char b, const char *fmt, ...)
{
    static char tmpbuf[512];
    va_list args;
    unsigned char *data = (unsigned char *)&g_dect.sct;
    unsigned int  maxbytes = sizeof(g_dect.sct);
    
    va_start(args, fmt);
    vsnprintf(tmpbuf,sizeof(tmpbuf)-1,fmt, args);
    va_end(args);
    showBinaryPos(GetBitsValue(data, maxbytes, a, b), 32, a, b, tmpbuf);
}

unsigned long long GetSptValue(unsigned char a, unsigned char b)
{
    return GetBitsValue((unsigned char *)&g_dect.spt, sizeof(g_dect.spt), a, b);
}

void ShowSpt(unsigned char a, unsigned char b, const char *fmt, ...)
{
    static char tmpbuf[512];
    va_list args;
    unsigned char *data = (unsigned char *)&g_dect.spt;
    unsigned int  maxbytes = sizeof(g_dect.spt);
    
    va_start(args, fmt);
    vsnprintf(tmpbuf,sizeof(tmpbuf)-1,fmt, args);
    va_end(args);
    showBinaryPos(GetBitsValue(data, maxbytes, a, b), 16, a, b, tmpbuf);
}

void BytesTrans(unsigned char *dst, unsigned char *src, int bytes)
{
    int i;
    for(i=0; i<bytes; i++)
    {
        dst[i] = src[bytes-i-1];
    }
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
int parseSpt(unsigned short spt)
{
    printf("---------------------------------------\n");
    printf("# Spt Data: %04X\n\n", spt);
    
    /* trans spt to network type */
    BytesTrans((unsigned char*)&g_dect.spt, (unsigned char*)&spt, sizeof(spt));
        
    /* Show spt */
    ShowSpt(0,0,"slot %s", GetSptValue(0,0)?"active":"un-active");
    ShowSpt(1,1,"uncond SPT INT=%d", GetSptValue(1,1));
    ShowSpt(2,15,"Address of Sct: 0x%08X", 0x00400000+GetSptValue(2,15));
    printf("\n\n");
    
    return 0;
}

int parseSct(unsigned int sct)
{
    printf("---------------------------------------\n");
    printf("# Sct Data: %08X\n\n", sct);
    
    /* trans sct to network type */
    BytesTrans((unsigned char*)&g_dect.sct, (unsigned char*)&sct, sizeof(sct));

    /* Show sct */
    ShowSct(0,0,"SCT ACT = %d", GetSctValue(0,0));
    ShowSct(1,1,"Uncond SCT INT = %d", GetSctValue(1,1));
    ShowSct(2,2,"Cond SCT INT = %d", GetSctValue(2,2));
    ShowSct(3,3,"ULE = %d", GetSctValue(3,3));
    ShowSct(4,4,"B_BYTE_ARRAY = %d", GetSctValue(4,4));
    ShowSct(5,6,"SLC_CTRL = %d", GetSctValue(5,6));
    ShowSct(7,7,"B_MUTE = %d", GetSctValue(7,7));
    ShowSct(8,8,"NO_SCRAMBLE = %d", GetSctValue(8,8));
    ShowSct(9,9,"CIPHER = %d", GetSctValue(9,9));
    ShowSct(10,10,"CHECK_ACCESS = %d", GetSctValue(10,10));
    ShowSct(11,12,"PACKET_TYPE = %d", GetSctValue(11,12));
    ShowSct(13,13,"PROTECTED = %d", GetSctValue(13,13));
    ShowSct(14,14,"TX = %d", GetSctValue(14,14));
    if(0 == GetSctValue(14,14))//Rx slot
        ShowSct(15,15,"SCT_COND_SEL = %d", GetSctValue(15,15));
    else //Tx slot
        ShowSct(15,15,"LONG_PREAMBLE = %d", GetSctValue(15,15));
    ShowSct(16,17,"SUBFIELD_TYPE = %d", GetSctValue(16,17));
    ShowSct(23,23,"SIF_LEN_SEL = %d", GetSctValue(23,23));
    ShowSct(24,24,"FP_PP_MODE = %d", GetSctValue(24,24));
    ShowSct(25,25,"FORCE_SYNC = %d", GetSctValue(25,25));
    ShowSct(26,26,"TIME_BASE = %d", GetSctValue(26,26));
    ShowSct(27,27,"T_DELAY = %d", GetSctValue(27,27));
    ShowSct(28,28,"NO_SYNC = %d", GetSctValue(28,28));
    ShowSct(29,29,"WIDE = %d", GetSctValue(29,29));
    ShowSct(30,30,"FAD = %d", GetSctValue(30,30));
    ShowSct(31,31,"ANT_SEL = %d", GetSctValue(31,31));
    printf("\n\n");
    
    return 0;
}

void stdinParse(void)
{
    char c;
    unsigned int type = 0;
    unsigned int ctrl = 0;
    int i;

    printf("Please input SPT info:\n");
    printf("> ");
    while(1)
    {
        printf("Please input ctrl type: 0=spt, 1=sct\n");
        printf("> ");
        if(1 != scanf("%d", &type))
        {
            printf("INPUT ERROR!\n");
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
        break;
    }
    
    g_dect.type = type;

    /* use Ctrl+C to break */
    while(1)
    {
        printf("Please input %s info:\n", type?"SCT":"SPT");
        printf("> ");
        if(1 != scanf("%08X", &ctrl))
        {
            printf("INPUT ERROR!\n");
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
        
        /* parase spt */
        switch(type)
        {
        case 0:
            parseSpt(ctrl);
            break;
        case 1:
            parseSct(ctrl);
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    FILE *inFile = NULL;
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
    }
    fclose(inFile);
    return 0;
}