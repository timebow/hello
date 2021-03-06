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
#define ST_DECT_DEV_NONE 0
#define ST_DECT_DEV_FP   1
#define ST_DECT_DEV_PP   2
#define ST_DECT_DIR_NONE 0
#define ST_DECT_DIR_TX   1
#define ST_DECT_DIR_RX   2

typedef struct
{
    char ifile[128];
    char ofile[128];
    int  len; 
    int  dev; //device
    int  dir; //direction
}ST_ARG;

ST_ARG g_arg = {0,};

typedef struct
{
    unsigned char dev;
    unsigned char dir;
    unsigned char afield[6];
}ST_DECT;

ST_DECT g_dect = {0};

// AField str index
const char *sAfHeadTa[]={
    "Ct Data packet number 0",
    "Ct Data packet number 1",
    "Nt postion",
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
    "Reserved",
    "Reserved"
};

const char *sAfTailQHxSP[]={ //Start Position
    "S-field starts at bit f0",
    "Reserved",
    "S-field starts at bit f240",
    "Reserved"
};

const char *sAfTailMtHead[]={
    "basic connection control",
    "advanced connection control",
    "MAC layer test messages",
    "quality control",
    "broadcast and connectionless services",
    "encryption control",
    "Tail for use with the first transmission of a B-field \"bearer request\" message",
    "escape",
    "TARI message",
    "REP connection control",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

const char *sAfTailMtH0Cmd[]={ //Mt.Head=0, basic connection control
    "access_request",
    "bearer_handover_request",
    "connection_handover_request",
    "unconfirmed_access_request",
    "bearer_confirm",
    "wait",
    "attributes_T_request",
    "attributes_T_confirm",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "release"
};

const char *sAfTailMtH1Cmd[]={ //Mt.Head=1, advanced connection control
    "ACCESS_REQUEST",
    "bearer_handover_request",
    "connection_handover_request",
    "unconfirmed_access_request",
    "bearer_confirm",
    "wait (contains FMID)",
    "attributes_T_request",
    "attributes_T_confirm",
    "bandwidth_T_request",
    "bandwidth_T_confirm",
    "channel_list",
    "unconfirmed_dummy",
    "unconfirmed_handover",
    "Reserved",
    "Reserved",
    "release"
};

const char *sAfTailMtH2Cmd[]={ //Mt.Head=2, MAC layer test messages 
    "FORCE_TRANSMIT",
    "LOOPBACK",
    "DEFEAT_ANTENNA_DIVERSITY",
    "reserved",
    "ESCAPE",
    "NETWORK_TEST",
    "CHANGE_MODULATION_SCHEME",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "CLEAR_TEST_MODES"
};

const char *sAfTailMtH3Cmd[]={ //Mt.Head=3, Quality control
    "antenna switch for the bearer(s) identified by LBN: request: PT --> FT, reject: FT --> PT",
    "antenna switch for all bearers of this connection to the RFP identified by its RPN: request: PT --> FT, reject: FT --> PT",
    "xxxx", //parase by sub field
    "connection handover: request: FT --> PT, reject: PT --> FT",
    "frequency control for the bearer identified by LBN: request: FT --> PT, reject: PT --> FT",
    "frequency control for all bearers of this connection to the RFP identified by its RPN: request: FT --> PT, reject: PT --> FT",
    "Advance timing for all the bearers of this connection to the RFP identified by its RPN: request: FT --> PT, reject: PT --> FT",
    "PT --> FT: PT informs that it is transmitting prolonged preamble in all the frames",
    "xxxx",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved"
};

const char *sAfTailPtHead[]={ //Pt.Head
    "zero length page ",
    "short page ",
    "full page",
    "MAC resume and control page",
    "not the last 36 bits of a long page",
    "the first 36 bits of a long page",
    "the last 36 bits of a long page",
    "all of a long page (first and last)"
};

const char *sAfTailPtHxMacInfo[]={ //Pt.Hx.MacInfo(b32-35)
    "fill bits / blind long slot (j=640/672) information",
    "blind full slot information for circuit mode service ",
    "other bearer",
    "recommended other bearer",
    "good RFP bearer",
    "dummy or C/L bearer position",
    "extended modulation types",
    "escape",
    "dummy or C/L bearer marker",
    "bearer handover/replacement information",
    "RFP status and modulation types",
    "active carriers",
    "C/L bearer position",
    "RFP power level",
    "blind double slot/RFP-FP interface resource information",
    "blind full slot information for packet mode service"
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
                if(sscanf(optarg,"0x%x",&g_arg.len)!=1)
                {
                    display_help();
                }
            }
            break;
            
        case 'v':
            {
                if(0 == strncmp(optarg, "fp", 2))
                {
                    g_arg.dev = ST_DECT_DEV_FP;
                }
                else if(0 == strncmp(optarg, "pp", 2))
                {
                    g_arg.dev = ST_DECT_DEV_PP;
                }
                else
                {
                    display_help();
                }
            }
            break;
            
        case 'r':
            {
                if(0 == strncmp(optarg, "tx", 2))
                {
                    g_arg.dir = ST_DECT_DIR_TX;
                }
                else if(0 == strncmp(optarg, "rx", 2))
                {
                    g_arg.dir = ST_DECT_DIR_RX;
                }
                else
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

long AFData(unsigned char a, unsigned char b)
{
    char len = b-a+1;
    char i, offsetByte = 0, offsetBit = 0;
    long curBit = 0;
    long val = 0;
    unsigned char *AField = g_dect.afield;
    
    if( (a > b) || (a > 47) || (b > 47) || (len > (sizeof(unsigned long)<<3)) )
    {
        printf("GetBits Error!");
        return -1;
    }
    
    for(i=a; i<=b; i++)
    {
        offsetByte = i>>3; //i/8;
        offsetBit = 7-(i&7); //7-(i%8);
        curBit = (AField[offsetByte] >> offsetBit) & 1;
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
void AFShow(unsigned char a, unsigned char b, const char *fmt, ...)
{
    static char tmpbuf[512];
    va_list args;
    int     len = 0;
    unsigned char *AField = g_dect.afield;
    
    va_start(args, fmt);
    len += vsnprintf(tmpbuf+len,sizeof(tmpbuf)-len-1,fmt, args);
    va_end(args);
    showBinaryPos(AFGetBitsValue(AField, a, b), 8, a, b, tmpbuf);
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

void QtMessage(unsigned char *AField)
{
    showBinary( AField[1]&0xF0, 8, 4, 7, sAfTailQHead[AField[1]>>4]);
    switch(AFData(8, 11))//( AField[1]&0xF0 )
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
    case 0x05:
        AFShow(12, 14, "ARI list length: %d", (AFData(12, 14)+1)<<1);
        AFShow(15, 15, "TARIs: %s", AFData(15, 15)?"yes":"no");
        AFShow(16, 16, "Black: %s", AFData(16, 16)?"yes":"no");
        AFShow(17, 47, "ARI or black-ARI: 0x%08X", AFData(17, 47));
        break;
    default:
        printf("un-parse Q%d Message!\n", AFData(8, 11));
        break;
    }
    
}

void MtMessage(unsigned char *AField)
{
    //a8-a11: Mt Header
    AFShow(8, 11, sAfTailMtHead[AFData(8, 11)]);
    switch( AFData(8, 11) )
    {
    case 0x00://Basic connection control
        AFShow(12, 15, sAfTailMtH0Cmd[AFData(12, 15)]);
        AFShow(16, 27, "FMID: %03X", AFData(16, 27));
        AFShow(28, 47, "PMID: %05lX", AFData(28, 47));
        break;
    case 0x01: //Advanced connection control
        AFShow(12, 15, sAfTailMtH1Cmd[AFData(12, 15)]);
        AFShow(16, 27, "FMID: %03X", AFData(16, 27));
        AFShow(28, 47, "PMID: %05lX", AFData(28, 47));
        break;
    case 0x02: //MAC layer test messages
        AFShow(12, 15, sAfTailMtH2Cmd[AFData(12, 15)]);
        AFShow(16, 47, "data: %08X", AFData(16, 47));
        break;
    case 0x03: //Quality control
        switch(AFData(12, 15))
        {
        case 0x00:
            AFShow(12, 15, sAfTailMtH3Cmd[AFData(12, 15)]);
            AFShow(16, 31, "LBN:%01X, LBN:%01X, LBN:%01X, LBN:%01X", AFData(16, 19), AFData(20, 23), AFData(24, 27), AFData(28, 31));
            AFShow(32, 47, "fix:%04X", AFData(32, 47));
            break;
        case 0x01:
            AFShow(12, 15, sAfTailMtH3Cmd[AFData(12, 15)]);
            AFShow(16, 31, "RPN:%02X, fix:%02X", AFData(16, 23), AFData(24, 31));
            AFShow(32, 47, "fix:%04X", AFData(32, 47));
            break;
        case 0x02:
            switch(AFData(16, 19))
            {
            case 0x0:
                AFShow(12, 15, "bearer handover/bearer replacement of the bearer identified by LBN: request: FT --> PT, reject: PT --> FT");
                AFShow(16, 31, "fix:%01X, LBN:%01X, fix/RPN:%02X", AFData(16, 19), AFData(20, 23), AFData(24, 31));
                break;
            case 0xF:
                AFShow(12, 15, "bearer handover/bearer replacement of the bearer identified by LBN: request: PT --> FT, reject: FT --> PT");
                AFShow(16, 31, "fix:%01X, LBN:%01X, fix/RPN:%02X", AFData(16, 19), AFData(20, 23), AFData(24, 31));
                break;
            default:
                printf("Mt.Quality.0x02.0x%X error!\n", AFData(16, 19));
                break;
            }
            AFShow(32, 47, "fix:%04X", AFData(32, 47));
            break;
        case 0x03:
            AFShow(12, 15, sAfTailMtH3Cmd[AFData(12, 15)]);
            AFShow(16, 47, "fix:%08X", AFData(16, 47));
            break;
        case 0x04:
            AFShow(12, 15, sAfTailMtH3Cmd[AFData(12, 15)]);
            AFShow(16, 31, "fix:%01X, LBN:%01X, freq err:%02X", AFData(16, 19), AFData(20, 23), AFData(24, 31));
            AFShow(32, 47, "fix:%04X", AFData(32, 47));
            break;
        case 0x05:
            AFShow(12, 15, sAfTailMtH3Cmd[AFData(12, 15)]);
            AFShow(16, 31, "RPN:%02X, freq err:%02X", AFData(16, 23), AFData(24, 31));
            AFShow(32, 47, "fix:%04X", AFData(32, 47));
            break;
        case 0x06:
            AFShow(12, 15, sAfTailMtH3Cmd[AFData(12, 15)]);
            AFShow(16, 31, "RPN:%02X, adv timing inc dec:%02X", AFData(16, 23), AFData(24, 31));
            AFShow(32, 47, "fix:%04X", AFData(32, 47));
            break;
        case 0x07:
            AFShow(12, 15, sAfTailMtH3Cmd[AFData(12, 15)]);
            AFShow(16, 31, "RPN:%02X, fix:%02X", AFData(16, 23), AFData(24, 31));
            AFShow(32, 47, "fix:%04X", AFData(32, 47));
            break;
        case 0x08:
            switch(AFData(16, 19))
            {
            case 0x0:
                AFShow(12, 15, "frequency replacement to carrier CN on slot pair SN: request PT -> FT, confirm FT -> PT");
                AFShow(16, 31, "fix:%01X, SN:%01X, fix:%01X, CN:%01X", AFData(16, 19), AFData(20, 23), AFData(24, 27), AFData(28, 31));
                break;
            case 0xF:
                AFShow(12, 15, "frequency replacement to carrier CN on slot pair SN: grant PT -> FT");
                AFShow(16, 31, "fix:%01X, SN:%01X, fix:%01X, CN:%01X", AFData(16, 19), AFData(20, 23), AFData(24, 27), AFData(28, 31));
                break;
            default:
                printf("Mt.Quality.0x08.0x%X error!\n", AFData(16, 19));
                break;
            }
            break;
        default:
            printf("Mt.Quality.reserved(0x%X) error!\n", AFData(12, 15));
            break;
        }
        break;
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
        //reserved!
        break;
    default:
        printf("un-parse Mt Message(0x%X)!\n", AFData(8, 11));
        break;
    }
}

void PtMessage(unsigned char *AField)
{
    //a8-a11: Pt Header
    AFShow(8, 8, AFData(8, 8)?"another page message shall start in the next frame in this multiframe that is permitted to contain a PT type tail."
                             :"the next occurrence of a normal page shall be in a frame ");
    AFShow(9, 11, "Bs Channel SDU length: %s", sAfTailPtHead[AFData(9, 11)]);
    switch( AFData(9, 11) )
    {
    /*
    case 0: //zero page
        AFShow(12, 31, "20 least significa bits of RFPI: 0x%05lX", AFData(12, 31));
        AFShow(32, 35, "info type: 0x%01X", AFData(32, 35));
        AFShow(36, 47, "MAC Layer information: 0x%03X", AFData(36, 47));
        break;
    case 1: //short page
        AFShow(12, 31, "20 bits of Bs channel data: 0x%05lX", AFData(12, 31));
        AFShow(32, 35, "info type: %X", AFData(32, 35));
        AFShow(36, 47, "MAC Layer information: 0x%03X", AFData(36, 47));
        */
    case 0: //zero page
    case 1: //short page
        AFShow(12, 31, "20 least significa bits of RFPI: 0x%05lX", AFData(12, 31));
        AFShow(32, 35, "info type: %X (%s)", AFData(32, 35), sAfTailPtHxMacInfo[AFData(32, 35)]);
        /* wireshark: cmac.afield.hdr.ta */
        switch(AFData(32, 35))
        {
        case 0x0: //longslot blindslots
            if(AFData(47, 47))
            {
                AFShow(36, 39, "fix fill bits: %X", AFData(36, 47));
            }
            else
            {
                int i;
                for(i=36;i<=47;i++) //long slot, 1/3/5.. always blind, so bit 47 spare = 0, also can be amused blind.
                {
                    AFShow(i, i, "long slot pair {%d,%d} is %s", i-36, i-24, AFData(i, i)?"NOT blind":"blind");
                }
                //AFShow(47, 47, "spare: %x", AFData(47, 47));
            }
            break;
        case 0x1: //blind full slot information for circuit mode service 
        case 0xF: //blind full slot information for packet mode service
            {
                int i;
                for(i=36;i<=47;i++)
                {
                    AFShow(i, i, "full slot pair {%d,%d} is %s", i-36, i-24, AFData(i, i)?"NOT blind":"blind");
                }
            }
            break;
        case 0x2: //other bearer 
        case 0x3: //recommended other bearer
        case 0x4: //good RFP bearer
        case 0x5: //dummy or connectionless bearer position
        case 0xA: //RFP status and modulation types
            //RFP status
            AFShow(39, 39, "RFP %s for speech", AFData(39, 39) ? "busy" : "clear");
            AFShow(38, 38, "system %s", AFData(38, 38) ? "busy" : "clear");
            AFShow(37, 37, "asynchronous FP %s", AFData(37, 37) ? "available" : "not available");
            AFShow(36, 36, "RFP %s for data", AFData(36, 36) ? "busy" : "clear");
            //A-field modulation scheme
            AFShow(43, 43, "2-level modulation %s", AFData(43, 43) ? "not supported" : "supported");
            AFShow(42, 42, "4-level modulation %s", AFData(42, 42) ? "not supported" : "supported");
            AFShow(41, 41, "8-level modulation %s", AFData(41, 41) ? "not supported" : "supported");
            if(AFData(40, 40))
            {
                AFShow(40, 40, "Reserved");
            }
            else
            {
                switch(AFData(40, 43))
                {
                case 0x1:
                    AFShow(40, 40, "Escape");
                    break;
                case 0x0:
                    AFShow(40, 40, "previous \"spare\" code: only 2-level modulation supported");
                    break;
                }
            }
            break;
        case 0xC: //C/L bearer position
            AFShow(36, 39, "SN(Slot Number): slot pair {%d,%d}", AFData(36, 39), AFData(36, 39)+12);
            AFShow(40, 41, "SP(Start Position): %s", sAfTailQHxSP[AFData(40, 41)]);
            AFShow(42, 47, "CN(Carrier Number): RF Carrier %d", AFData(42, 47));
            break;
        default:
            printf("unknown page info.\n");
            break;
        }
        break;
    case 2: //full page
        AFShow(12, 47, "36 bits of Bs channel data: 0x%09lX", AFData(12, 47));
        break;
    case 3: //MAC resume and control page
        AFShow(12, 31, "PMID: 0x%05lX", AFData(12, 31));
        AFShow(32, 35, "ECN/Info 3: 0x%01X", AFData(32, 35));
        AFShow(36, 37, "Command: 0x%X", AFData(36, 37));
        AFShow(38, 41, "Info 1: 0x%X", AFData(38, 41));
        AFShow(42, 47, "Info 2: 0x%X", AFData(42, 47));
        break;
    case 4: //not the last 36 bits of a long page
	    AFShow(12, 47, "(Page WITHOUT data)");
        break;
    case 5: //the first 36 bits of a long page 
    case 6: //the last 36 bits of a long page
    case 7: //all of a long page (first and last) 
        AFShow(12, 47, "36 bits of Bs channel data: 0x%09lX", AFData(12, 47));
        break;
    default:
        printf("un-parse Pt Message!\n");
        break;
    }
}

int parseAField(unsigned char *AField)
{
    printf("---------------------------------------\n");
    printf("# AFiled Data: %02X %02X %02X %02X %02X %02X\n\n", 
        AField[0], AField[1], AField[2], AField[3], AField[4], AField[5]);
    
    /* Store afield to use */
    memcpy(g_dect.afield, AField, 6);
    
    /* Show Head */
    AFHead( AField );
    /* A0-A2 */
    switch( AFData(0, 2) )
    {
    case 0: //Ct0
    case 1: //Ct1
        AFShow(8, 47, "CT#%d: %010lX", AFData(0, 2), AFData(8, 47)); //Ct Data is for DLC, mulit-segments combined into complete data(MID_MAC_CO_DATA_IND)
        break;
    case 2: //Nt position
        if(AFData(4, 6) == 7)
            printf("Nt is on dummy bearer!(BA=111)\n");
        else
            printf("Nt is on connectionless bearer!(BA!=111)\n");
        break;
    case 3: //Nt
        AFShow(8, 47, "RFPI: %010lX", AFData(8, 47));
        break;
    case 4: //Qt
        QtMessage( AField );
        break;
    case 5: //reserved
        printf("reserved!\n");
        break;
    case 6: //Mt
        MtMessage( AField );
        break;
    case 7: //First-Mt(PP) or Pt
        if ( (g_dect.dev == ST_DECT_DEV_FP && g_dect.dir == ST_DECT_DIR_TX) || (g_dect.dev == ST_DECT_DEV_PP && g_dect.dir == ST_DECT_DIR_RX) )//Pt
        {
            PtMessage( AField );
        }
        else if ( (g_dect.dev == ST_DECT_DEV_FP && g_dect.dir == ST_DECT_DIR_RX) || (g_dect.dev == ST_DECT_DEV_PP && g_dect.dir == ST_DECT_DIR_TX) )//Mt
        {
            MtMessage( AField );
        }
        else //unknown device & direction, then print all case.
        {
            if(AFData(28, 47) < 8)
            {
                printf("\n>>>>> If From PP: First Mt (Auto Check: %s possiblity)\n", AFData(28, 47) < 8 ? "High":"Low");
                MtMessage( AField );
                printf("\n>>>>> If From FP: Pt (Auto Check: %s possiblity)\n", AFData(28, 47) >= 8 ? "High":"Low");
                PtMessage( AField );
            }
            else
            {
                printf("\n>>>>> If From FP: Pt (Auto Check: %s possiblity)\n", AFData(28, 47) >= 8 ? "High":"Low");
                PtMessage( AField );
                printf("\n>>>>> If From PP: First Mt (Auto Check: %s possiblity)\n", AFData(28, 47) < 8 ? "High":"Low");
                MtMessage( AField );
            }
        }
        break;
    default:
        printf("un-known AField!\n");
        break;
    }
    printf("\n\n");
    
    return 0;
}

void stdinParse(void)
{
    unsigned char AField[5];
    char c;
    unsigned int wBlindInfo = 0;
    unsigned int wEvenBlind = 0xAAA;
    unsigned int wOddBlind = 0x555;
    char bBlindCnt;
    char bNeedMarker = 0;
    int i;
    
    /* use Ctrl+C to break */
    while(1)
    {
        printf("Please input blind info:\n");
        printf("> ");
        if(1 != scanf("%08X", &wBlindInfo))
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
        
        /* blind slot */
        printf("input 0x%03X\n", wBlindInfo);
        printf(" => ");
        bBlindCnt = 0;
        for(i=0; i<12; i++)
        {
            if(0 == (wBlindInfo & (1<<(11-i))))
            {
                bBlindCnt++;
                if((wBlindInfo & wEvenBlind) == 0)
                {
                    bNeedMarker = (i&1); //if odd
                }
                else if((wBlindInfo & wOddBlind) == 0)
                {
                    bNeedMarker = (i&1)==0; //if even
                }
                else 
                {
                    bNeedMarker = 0;
                }
                
                if(bNeedMarker)
                {
                    if(bBlindCnt == 1)
                        printf("(%d)", i);
                    else
                        printf("/(%d)", i);
                }
                else 
                {
                    if(bBlindCnt == 1)
                        printf("%d", i);
                    else
                        printf("/%d", i);
                }
            }
        }
        printf(" blind\n\n");
    }
}

int main(int argc, char *argv[])
{
    FILE *inFile = NULL;
    unsigned char AField[5];
    char tmpstr[1024];

    /* parse options */
    parse_options(argc, (const char **)argv);
    
    /* params for dect */
    g_dect.dev = g_arg.dev;
    g_dect.dir = g_arg.dir;
    
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
        /*读到本行结束*/
        fgets(tmpstr, 1024 , inFile);
        /*解析AField*/
        parseAField(AField);
    }
    fclose(inFile);    
    return 0;
}