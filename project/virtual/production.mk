# main project for production
MODULES += \
	app/shell

MODULES += lib/vfs

include project/virtual/test.mk
include project/virtual/fs.mk
include project/virtual/minip.mk

