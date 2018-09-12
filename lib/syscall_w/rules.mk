LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/syscall.c

MODULE_DEPS := \
    lib/vstream

include make/module.mk
