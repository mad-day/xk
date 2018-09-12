# main project for qemu-aarch64

MODULES += lib/syscall_w/arm64

include project/virtual/production.mk
include project/target/qemu-virt-a53.mk

