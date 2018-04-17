#
# 说明:
# 自动搜索全部子目录, 编译各级目录下的c文件到obj, 然后完成连接.
#

CC=g++

ROOT=.
PROJDIR=$(ROOT)/..
OBJDIR=$(ROOT)/obj
TARGET=$(OBJDIR)/client

SRCEXT := .c 
#不要搜索的目录
EXCLUDE_DIRS := obj
#搜寻当前目录下的所有目录
SRCDIRS := $(filter-out $(EXCLUDE_DIRS),$(basename $(patsubst ./%,%,$(shell find . -type d))))
#搜寻所有目录下所有.c的文件
SRCS	:= $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*, $(SRCEXT))))
file:=$(notdir $(SRCS))
OBJS:=$(patsubst %.c,$(OBJDIR)/%.o,$(file))
temp:=$(filter %BusUart.c,$(SRCS))

CFLAGS+=$(INCS)

.PHONY: all clean

all:$(TARGET)

$(TARGET):$(OBJS)  
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) -c $(filter %$(patsubst %o,%c,$(notdir $@)),$(SRCS)) -o $@
        
clean:
	@echo "make" $(TARGET) "clean"
	rm -f $(OBJDIR)/*.a $(OBJDIR)/*.o
