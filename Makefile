TARGET = ctrlhook
OBJS = main.o

# Define to build this as a prx (instead of a static elf)
BUILD_PRX = 1

USE_KERNEL_LIBS = 1
USE_KERNEL_LIBC = 1

CFLAGS = -O2 -G0 -w -msingle-float -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti  
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -lc -lpspuser -lpspdebug -lpspge libs/libpspsystemctrl_user.a libs/libpspsystemctrl_kernel.a libs/libpspkubridge.a
LDFLAGS = -nostartfiles 

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak 