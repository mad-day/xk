LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += $(LOCAL_DIR)/arm64.c

MODULE_DEPS := lib/syscall_w

include make/module.mk
