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

typedef union
{
    unsigned int code;
    struct
    {
        unsigned int shifter_operand:12;
        unsigned int Rd:4;
        unsigned int Rn:4;
        unsigned int S:1;
        unsigned int opcode:4;
        unsigned int I:1;
        unsigned int rsv:2; //reserved
        unsigned int cond:4;
    };
}ST_CODE;

ST_CODE g_code = {0,};


const char *sCond[]={
    "EQ",
    "NE",
    "CS/HS",
    "CC/LO",
    "MI",
    "PL",
    "VS",
    "VC",
    "HI",
    "LS",
    "GE",
    "LT",
    "GT",
    "LE",
    "", //AL,not show
    "-"
};

const char *sOpcode[]={
    "AND",
    "EOR",
    "SUB",
    "RSB",
    "ADD",
    "ADC",
    "SBC",
    "RSC",
    "TST",
    "TEQ",
    "CMP",
    "CMN",
    "ORR",
    "MOV",
    "BIC",
    "MVN"
};

const char *sRx[]={
    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "PC"
};

enum
{
   OPCODE_AND,
   OPCODE_EOR,
   OPCODE_SUB,
   OPCODE_RSB,
   OPCODE_ADD,
   OPCODE_ADC,
   OPCODE_SBC,
   OPCODE_RSC,
   OPCODE_TST,
   OPCODE_TEQ,
   OPCODE_CMP,
   OPCODE_CMN,
   OPCODE_ORR,
   OPCODE_MOV,
   OPCODE_BIC,
   OPCODE_MVN
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

long CodeData(unsigned char a, unsigned char b)
{
    char len = b-a+1;
    char i, offsetByte = 0, offsetBit = 0;
    long curBit = 0;
    long val = 0;
    unsigned char *code = (unsigned char *)&g_code.code;
    
    if( (a > b) || (a > 31) || (b > 31) || (len > (sizeof(unsigned long)<<3)) )
    {
        printf("GetBits Error!");
        return -1;
    }
    
    for(i=a; i<=b; i++)
    {
        offsetByte = i>>3; //i/8;
        offsetBit = 7-(i&7); //7-(i%8);
        curBit = (code[offsetByte] >> offsetBit) & 1;
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
void CodeShow(unsigned char a, unsigned char b, const char *fmt, ...)
{
    static char tmpbuf[512];
    va_list args;
    int     len = 0;
    unsigned char *code = (unsigned char *)&g_code.code;
    
    va_start(args, fmt);
    len += vsnprintf(tmpbuf+len,sizeof(tmpbuf)-len-1,fmt, args);
    va_end(args);
    showBinaryPos(CodeGetBitsValue(code, a, b), 8, a, b, tmpbuf);
}

/*
https://www.cnblogs.com/51qianrushi/p/4614491.html
<opcode1>{<cond>}{S} <Rd>,<shifter_operand>
<opcode1>:=MOV|MVN
<opcode2>{<cond>} <Rn>,<shifter_operand>
<opcode2>:=CMP|CMN|TST|TEQ
<opcode3>{<cond>}{S} <Rd>,<Rn>,<shifter_operand>
<opcode3>:=ADD|SUB|RSB|ADC|SBC|RSC|AND|BIC|EOR|ORR
*/
int parseArmCode(unsigned int code)
{
    printf("---------------------------------------\n");
    printf("#Code: %08X\n", code);
    printf("#Symb: ");
    
    /* Store afield to use */
    g_code.code = code;
    
    switch(g_code.rsv)
    {
    case 0:  //normal code
        switch(g_code.opcode)
        {
        case OPCODE_MOV:
        case OPCODE_MVN:
            if(g_code.I)
                printf("%s%s%s %s,#%X", sOpcode[g_code.opcode], sCond[g_code.cond], g_code.S?"S":"", sRx[g_code.Rd], g_code.shifter_operand);
            else
                printf("%s%s%s %s,%s", sOpcode[g_code.opcode], sCond[g_code.cond], g_code.S?"S":"", sRx[g_code.Rd], sRx[g_code.shifter_operand]);
            break;
        case OPCODE_CMP:
        case OPCODE_CMN:
        case OPCODE_TST:
        case OPCODE_TEQ:
            if(g_code.I)
                printf("%s%s %s,#%X", sOpcode[g_code.opcode], sCond[g_code.cond], sRx[g_code.Rn], g_code.shifter_operand);
            else
                printf("%s%s %s,%s", sOpcode[g_code.opcode], sCond[g_code.cond], sRx[g_code.Rn], sRx[g_code.shifter_operand]);
            break;
        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_RSB:
        case OPCODE_ADC:
        case OPCODE_SBC:
        case OPCODE_RSC:
        case OPCODE_AND:
        case OPCODE_BIC:
        case OPCODE_EOR:
        case OPCODE_ORR:
            if(g_code.I)
                printf("%s%s%s %s,%s,#%X", sOpcode[g_code.opcode], sCond[g_code.cond], g_code.S?"S":"", sRx[g_code.Rd], sRx[g_code.Rn], g_code.shifter_operand);
            else
                printf("%s%s%s %s,%s,%s", sOpcode[g_code.opcode], sCond[g_code.cond], g_code.S?"S":"", sRx[g_code.Rd], sRx[g_code.Rn], sRx[g_code.shifter_operand]);  
            break;
        }
        break;
    default: //new code
        printf("Unkknown code!");
        break;
    }
    printf("\n\n");
    
    return 0;
}

void stdinParse(void)
{
    unsigned int armcode;
    char c;
    int ret;
    /* use Ctrl+C to break */
    while(1)
    {
        printf("Please input arm code(1 DWORD):\n");
        printf("> ");
        if(1 != (ret= scanf("%08X", &armcode)))
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
        
        /*解析AField*/
        parseArmCode(armcode);
    }
}

int main(int argc, char *argv[])
{
    FILE *inFile = NULL;
    unsigned int code;
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
        fscanf(inFile,"%08X", &code);
        /*读到本行结束*/
        fgets(tmpstr, 1024 , inFile);
        /*解析AField*/
        parseArmCode(code);
    }
    fclose(inFile);    
    return 0;
}