LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/vfs.c \
	$(LOCAL_DIR)/vfs_vndef.c \
	$(LOCAL_DIR)/vfs_vfsdef.c \
	$(LOCAL_DIR)/vfs_vnops.c \
	$(LOCAL_DIR)/vfs_alloc.c

EXTRA_LINKER_SCRIPTS += $(LOCAL_DIR)/vfs.ld

MODULE_DEPS := \
    lib/vstream \
    lib/zalloc

include make/module.mk
