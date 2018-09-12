# main project for qemu-arm32

MODULES += lib/syscall_w/arm

include project/virtual/production.mk
include project/target/qemu-virt-a15.mk

