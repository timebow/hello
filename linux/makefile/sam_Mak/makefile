################################################################################
SL := /
PRJ_DIR := .
OBJ_DIR := .$(SL)obj
TARGET_DIR := .$(SL)out

OBJS := 
SRCS := 
INCS := 

################################################################################
# include module "main"
include $(PRJ_DIR)$(SL)main$(SL)main.mak
# include module "hello"
include $(PRJ_DIR)$(SL)module_hello$(SL)hello.mak

################################################################################
# path of header files
CFLAGS += $(INCS)

################################################################################
TARGETS := $(TARGET_DIR)$(SL)test

################################################################################
# it's better to declare all fake targets(NOT real files), to avoid confict with those files with same name. 
.PHONY:all clean

all:$(TARGETS)
#$(TARGETS) need $(OBJS), and do "gcc -o $@ $(OBJS)" to produce $(TARGETS)
$(TARGETS):$(OBJS)
	@gcc -o $@ $(OBJS)
	@echo build done.
#$(OBJS) need $(SRCS), and do "gcc -c $(SRCS) -o $@" to produce $(OBJS)
$(OBJS):$(SRCS)
	@gcc $(CFLAGS) -c $(filter %$(patsubst %o,%c,$(notdir $@)),$(SRCS)) -o $@

clean:
	@rm $(TARGETS) $(OBJS)
	@echo clean done.