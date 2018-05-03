/*! \mainpage generate 数据生成器
 *	\section About 关于
 *  
 *  \n 版本: 
 *  \n 用途: 生成规律数据文件
 *  \n 语言: C (ANSI)
 *  \n 平台: LINUX
 *  \n 编译: Gcc
 *  \n 作者: 刘元龙
 *
 *  \section Log 日志
 *  \n 2018 v0.1 初版
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
    char gen[128];
    char type[128];
    unsigned int start;
    unsigned int min;
    unsigned int max;
    unsigned int size;
    unsigned int count;
    unsigned int step;
}ST_ARG;

ST_ARG g_arg =
{
    "",
    "",
    "inc",
    "bin",
    0,
    0,
    0xFFFFFFFF,
    4,
    0,
    1
};

/************************************************************************/
/*                                                                      */
/************************************************************************/
static void display_help (void)
{
    printf("\nExample: test -c 10 -o ./test.bin\n");
    printf("\n"
        "[" __DATE__ " " __TIME__ "]\n"
        "        --help            display this help and exit\n"
        "        --version         output version information and exit\n"
        "        --gen             gen data type, rand/inc (default inc)\n"
        "        --min             min data for rand gen type (default 0)\n"
        "        --max             max data for rand gen type (default 0xFFFFFFFF)\n"
        "        --start           start value for inc gen type (default 0)\n"
        "        --size            size(1/2/4B) for inc gen type (default 4B)\n"
        "-c      --count           count for inc gen type\n"
        "-s      --step            step for inc gen type(default 1)\n"
        "-i      --ifile           input file path\n"
        "-o      --ofile           output file path\n"
        "-t      --type            data type: bin/txt (default bin)\n"
        "\n"
        );
    exit(0);
}

static int parse_options(int argc, const char *argv[])
{
    for (;;) 
    {
        int option_index = 0;
        static const char  *short_options = "c:s:i:o:t:";
        static const struct option long_options[] = 
        {
            /*server common define*/
            {"help",            no_argument,        0, 0},
            {"version",         no_argument,        0, 0},

            /*application define*/
            {"gen",             required_argument,        0, 0},//6
            {"min",             required_argument,        0, 0},
            {"max",             required_argument,        0, 0},
            {"start",           required_argument,        0, 0},
            {"size",            required_argument,        0, 0},
            {"count",           required_argument,        0, 'c'},
            {"step",            required_argument,        0, 's'},
            {"ifile",           required_argument,        0, 'i'},
            {"ofile",           required_argument,        0, 'o'},
            {"type",            required_argument,        0, 't'},
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
            case 6: //gen
                strncpy(g_arg.ifile, optarg, 128);
                break;
            case 7: //min
                g_arg.min = strtol ( optarg, ( char ** ) NULL, 0 );
                break;
            case 8: //max
                g_arg.max = strtol ( optarg, ( char ** ) NULL, 0 );
                break;
            case 9: //start
                g_arg.start = strtol ( optarg, ( char ** ) NULL, 0 );
                break;
            case 10: //size
                g_arg.size = strtol ( optarg, ( char ** ) NULL, 0 );
                break;
            case 11: //count
                g_arg.count = strtol ( optarg, ( char ** ) NULL, 0 );
                break;
            case 12: //step
                g_arg.step = strtol ( optarg, ( char ** ) NULL, 0 );
                break;
            case 13: //ifile
                strncpy(g_arg.ifile, optarg, 128);
                break;
            case 14: //ofile
                strncpy(g_arg.ofile, optarg, 128);
                break;
            case 15: //type
                strncpy(g_arg.type, optarg, 128);
                break;
            }
            break;

        case 'c':
            {
                g_arg.count = strtol ( optarg, ( char ** ) NULL, 0 );
                break;
            }
            break;

        case 's':
            {
                g_arg.step = strtol ( optarg, ( char ** ) NULL, 0 );
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
            
        case 't':
            {
                strncpy(g_arg.type, optarg, 128);
                break;
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

int main(int argc, char *argv[])
{
    FILE *inFile = NULL;
    FILE *outFile = NULL;
    unsigned char  vbyte  = (unsigned char )g_arg.start;
    unsigned short vshort = (unsigned short)g_arg.start;
    unsigned int   vint   = (unsigned int  )g_arg.start;
    unsigned int   i = 0;

    /* parse options */
    if(argc <= 1) display_help();
    parse_options(argc, (const char **)argv);

    if(0 == strcmp(g_arg.gen,"rand"))
    {
        printf("Generate: gen type:%s, data type:%s, min=0x%X, max=0x%X, count=%d(0x%X)\n", g_arg.gen, g_arg.type, g_arg.min, g_arg.max, g_arg.count, g_arg.count);
        printf("Not support rand now!\n");
    }
    else if(0 == strcmp(g_arg.gen,"inc"))
    {
        printf("Generate: gen type:%s, data type:%s, start=0x%X, size=%d, count=%d(0x%X), step=%d\n", g_arg.gen, g_arg.type, g_arg.start, g_arg.size, g_arg.count, g_arg.count, g_arg.step);
        if(0 == strcmp(g_arg.type,"bin"))
        {
            if(g_arg.count == 0)
            {
                printf("ERROR: count=%d!\n", g_arg.count);
                return -1;
            }
            if(g_arg.step == 0)
            {
                printf("ERROR: step=%d!\n", g_arg.step);
                return -1;
            }

            if((outFile=fopen(g_arg.ofile,"wb"))==NULL)
            {
                printf("Ouput file error: %s\n",g_arg.ofile);
                display_help();
                return -1;
            }

            switch(g_arg.size)
            {
            case 1://u8
                for(i=0;i<g_arg.count;i++)
                {  
                    fwrite(&vbyte,1,1,outFile);
                    vbyte += (unsigned char)g_arg.step;
                }
                break;
            case 2://u16
                for(i=0;i<g_arg.count;i++) 
                {  
                    fwrite(&vshort,2,1,outFile);
                    vshort += (unsigned short)g_arg.step;
                }
                break;
            case 4://u32
                for(i=0;i<g_arg.count;i++) 
                {  
                    fwrite(&vint,4,1,outFile);
                    vint += (unsigned int)g_arg.step;
                }
                break;
            default:
                printf("size=%dB, NOT support or error!\n", g_arg.size);
                break;
            }
            fclose(outFile);
        }
        else if(0 == strcmp(g_arg.type,"txt"))
        {       
            if((outFile=fopen(g_arg.ofile,"wb"))==NULL)
            {
                printf("Ouput file error: %s\n",g_arg.ofile);
                display_help();
                return -1;
            }

            switch(g_arg.size)
            {
            case 1://u8
                for(i=0;i<g_arg.count;i++) 
                {
                    fprintf(outFile, "0x%02X\n", vbyte);
                    vbyte += g_arg.step;
                }
                break;
            case 2://u16
                for(i=0;i<g_arg.count;i++) 
                {
                    fprintf(outFile, "0x%04X\n", vshort);
                    vshort += g_arg.step;
                }
                break;
            case 4://u32
                for(i=0;i<g_arg.count;i++) 
                {
                    fprintf(outFile, "0x%08X\n", vint);
                    vint += g_arg.step;
                }
                break;
            default:
                printf("size=%dB, NOT support or error!\n", g_arg.size);
                break;
            }
            fclose(outFile);
        }
        else
        {
            printf("Not Support data type %s!!!\n", g_arg.type);
        }
    }
    else 
    {
        printf("Not Support gen type %s!!!\n", g_arg.gen);
    }
    return 0;
}