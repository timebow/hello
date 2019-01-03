M_HELLO_DIR := $(PRJ_DIR)$(SL)module_hello

# include
INCS += -I$(M_HELLO_DIR)

# source files
SRCS += $(M_HELLO_DIR)$(SL)hello.c

# obj files
OBJS += $(OBJ_DIR)$(SL)hello.o

vpath %.c $(M_HELLO_DIR)
vpath %.h $(M_HELLO_DIR)