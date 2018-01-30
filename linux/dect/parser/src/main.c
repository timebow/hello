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

// AField str index
const char *sAfHeadTa[]={
    "Ct Data packet number 0",
    "Ct Data packet number 1",
    "Nt",
    "Nt",
    "Qt",
    "reserved",
    "Mt",
    "Pt(RFP)/Mt-First(PP)"
};

const char *sAfTailQHead[]={
    "static system info 0. (normal RFP transmit half frame)",
    "static system info 1. (normal PP transmit half frame)",
    "extended RF carriers part 1",
    "fixed part capabilities",
    "extended fixed part capabilities",
    "SARI list contents ",
    "multi-frame number",
    "escape",
    "obsolete",
    "extended RF carriers part 2",
    "transmit information",
    "extended fixed part capabilities part 2",
    "Reserved",
    "Reserved",
    "Reserved"
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

/*****************************************************************************************
* @brief  : get value from offset a to offset b.
* @param  : AField: AField data
            a     : start bit offset, A0-A47, bit offset as ETSI used.
            b     : end bit offset, A0-A47, bit offset as ETSI used
* @warning: long may be 4/8 bytes
* @return : -1=fail, others=value of a->b.
******************************************************************************************/
long AFGetBitsValue(unsigned char *AField, unsigned char a, unsigned char b)
{
    char len = b-a+1;
    char i, offsetByte = 0, offsetBit = 0;
    long curBit = 0;
    long val = 0;
    
    if( !AField || (a > b) || (a > 47) || (b > 47) || (len > (sizeof(unsigned long)<<3)) )
    {
        printf("GetBits Error!");
        return -1;
    }
    
    for(i=a; i<=b; i++)
    {
        offsetByte = i/8;
        offsetBit = 7-(i%8);
        curBit = (AField[offsetByte] >> offsetBit) & 1;
        val |= curBit<<(b-i);
    }
    
    return val;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void AFHead(unsigned char *AField)
{
    showBinary( AField[0]&0xE0, 8, 5, 7, sAfHeadTa[AField[0]>>5] );
    showBinary( AField[0]&0x10, 8, 4, 4, "Q1/BCK" );
    showBinary( AField[0]&0x0E, 8, 1, 3, "BA" );
    showBinary( AField[0]&0x01, 8, 0, 0, "Q2" );
}

void QMessage(unsigned char *AField)
{
    showBinary( AField[1]&0xF0, 8, 4, 7, sAfTailQHead[AField[1]>>4]);
    switch( AField[1]&0xF0 )
    {
    case 0x00:
    case 0x01:
        showBinary( AField[1]&0x0F, 8, 0, 3, "slot pair {%d,%d}", AField[1]&0x0F, (AField[1]&0x0F)+12);
        showBinary( AField[2]&0xC0, 8, 6, 7, "SP");
        showBinary( AField[2]&0x20, 8, 5, 5, "ESC");
        showBinary( AField[2]&0x18, 8, 3, 4, "Txs");
        showBinary( AField[2]&0x04, 8, 2, 2, "Mc:%s",AField[2]&0x04?"extended carrier info":"NOT extended carrier info");
        showBinary( AField[2]&0x02, 8, 1, 1, "Carrier0 %s",AField[2]&0x02?"available":"NOT available"); //bit 22-31
        showBinary( AField[2]&0x01, 8, 0, 0, "Carrier1 %s",AField[2]&0x01?"available":"NOT available");
        showBinary( AField[3]&0x80, 8, 7, 7, "Carrier2 %s",AField[3]&0x80?"available":"NOT available");
        showBinary( AField[3]&0x40, 8, 6, 6, "Carrier3 %s",AField[3]&0x40?"available":"NOT available");
        showBinary( AField[3]&0x20, 8, 5, 5, "Carrier4 %s",AField[3]&0x20?"available":"NOT available");
        showBinary( AField[3]&0x10, 8, 4, 4, "Carrier5 %s",AField[3]&0x10?"available":"NOT available");
        showBinary( AField[3]&0x08, 8, 3, 3, "Carrier6 %s",AField[3]&0x08?"available":"NOT available");
        showBinary( AField[3]&0x04, 8, 2, 2, "Carrier7 %s",AField[3]&0x04?"available":"NOT available");
        showBinary( AField[3]&0x02, 8, 1, 1, "Carrier8 %s",AField[3]&0x02?"available":"NOT available");
        showBinary( AField[3]&0x01, 8, 0, 0, "Carrier9 %s",AField[3]&0x01?"available":"NOT available");
        showBinary( AField[4]&0xC0, 8, 6, 7, "Spare");//bit 32-33
        showBinary( AField[4]&0x3F, 8, 0, 5, "Carrier number: %d", AField[4]&0x3F);//bit 34-39
        showBinary( AField[5]&0xC0, 8, 6, 7, "Spare");//bit 40-41
        showBinary( AField[5]&0x3F, 8, 0, 5, "PSCN: %d", AField[5]&0x3F);//bit 42-47
        break;
    default:
        printf("un-parse Q Message!\n");
        break;
    }
    
}

int parseAField(unsigned char *AField)
{
    printf("AFiled: %02X %02X %02X %02X %02X %02X\n", 
        AField[0], AField[1], AField[2], AField[3], AField[4], AField[5]);
    
    /* Show Head */
    AFHead( AField );
    /* A0-A2 */
    switch( AFGetBitsValue(AField, 0, 2) )
    {
    case 0:
        printf("un-parse Ct!\n");
        break;
    case 1:
        printf("un-parse Ct!\n");
        break;
    case 2:
        printf("un-parse Nt!\n");
        break;
    case 3:
        printf("un-parse Nt!\n");
        break;
    case 4:
        QMessage( AField );
        break;
    case 5:
        printf("reserved!\n");
        break;
    case 6:
        printf("un-parse Mt!\n");
        break;
    case 7:
        printf("un-parse Pt/Mt!\n");
        break;
    default:
        printf("un-known AField!\n");
        break;
    }
    printf("\n");
    
    return 0;
}

void stdinParse(void)
{
    unsigned char AField[5];
    char c;
    
    /* use Ctrl+C to break */
    while(1)
    {
        printf("Please input Afiled data (6 Bytes):\n");
        if(6 != scanf("%02x%*c%02x%*c%02x%*c%02x%*c%02x%*c%02x", &AField[0], &AField[1], &AField[2], &AField[3], &AField[4], &AField[5]))
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
        
        /*解析AField*/
        parseAField(AField);
    }
}

int main(int argc, char *argv[])
{
    FILE *inFile = NULL;
    unsigned char AField[5];

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
        fscanf(inFile,"%02x%*c%02x%*c%02x%*c%02x%*c%02x%*c%02x", &AField[0], &AField[1], &AField[2], &AField[3], &AField[4], &AField[5]);
        /*解析AField*/
        parseAField(AField);
    }
    fclose(inFile);    
    return 0;
}