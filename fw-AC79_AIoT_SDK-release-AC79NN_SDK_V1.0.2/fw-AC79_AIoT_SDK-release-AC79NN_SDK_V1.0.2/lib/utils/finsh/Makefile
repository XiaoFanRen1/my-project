
AR_OUT = $(AR_DIR)/finsh.a

CC_DEFINE += \
-DRT_USING_FINSH \
-DFINSH_USING_SYMTAB \
-DFINSH_USING_DESCRIPTION \
-DFINSH_USING_HISTORY \
-DFINSH_USING_MSH \
-DFINSH_USING_MSH_DEFAULT \


includes += \

objs += \
    cmd.o \
    finsh_compiler.o \
    finsh_error.o \
    finsh_heap.o \
    finsh_init.o \
    finsh_node.o \
    finsh_ops.o \
    finsh_parser.o \
    finsh_token.o \
    finsh_vm.o \
    finsh_var.o \
    msh.o \
    shell.o \
    symbol.o \
    msh_file.o \

objs_ver = version.o

GEN_LIB=y

include $(MAKE_RULE)
