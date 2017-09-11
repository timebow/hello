
CC=g++

ROOT=.
PROJDIR=$(ROOT)/..
OBJDIR=$(ROOT)/obj
TARGET=$(OBJDIR)/test

exclude_dirs:=obj
SUBDIR:=$(filter-out $(exclude_dirs),$(basename $(patsubst ./%,%,$(shell find . -maxdepth 1 -type d))))
#incs:=$(foreach dir,$(SUBDIR),$(shell ls $(dir)/inc/*.h))
#SRCS:=$(foreach dir,$(SUBDIR),$(shell ls $(dir)/*.c))

SRCEXT := .c 
#搜寻当前目录下的所有目录
SRCDIRS := $(patsubst ./%,%,$(shell find . -type d))
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
