M_MAIN_DIR := $(PRJ_DIR)$(SL)main

# include
INCS += -I$(M_MAIN_DIR)

# source files
SRCS += $(M_MAIN_DIR)$(SL)main.c

# obj files
OBJS += $(OBJ_DIR)$(SL)main.o

vpath %.c $(M_MAIN_DIR)
vpath %.h $(M_MAIN_DIR)