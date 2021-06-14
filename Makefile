CROSS_COMPILE = arm-none-eabi-

CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
OBJCOPY		= $(CROSS_COMPILE)objcopy
CFLAGS		= 
LDFLAGS     = -emain --script=spl.lds --trace --verbose=5
OBJCOPYFLAGS= --strip-all

LIBGCC := -L$(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -L/usr/lib/arm-none-eabi/lib -static -lgcc -lc
LIBS += $(LIBGCC)

INCLUDE     =

SRCDIR += .      
SRCDIR += include
SRCDIR += include/common
SRCDIR += include/asm
SRCDIR += include/init_dram
SRCDIR += include/uart
SRCDIR += include/gpio
SRCDIR += include/interinc
SRCDIR += tools

LDFLAGS += $(LIBS)

TARGET		:= out.bin
CONFIG_SPL_TEXT_BASE 	= 
CONFIG_SPL_MAX_SIZE 	=

INCLUDE += $(foreach dir,$(SRCDIR),-I$(dir))

SRCCS += $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.c))
SRCSS += $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.s))
SRCSS += $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*.S))
OBJS  = ${SRCCS:.c=.o} $(SRCSS:.s=.o) $(SRCSS:.S=.o)
SRCS  = ${SRCCS} $(SRCSS)
TARGET_ELF = elf-$(TARGET)

all:$(TARGET)

$(TARGET):$(TARGET_ELF) mksunxiboot
	$(OBJCOPY) --gap-fill=0xff $(OBJCOPYFLAGS) $< $@

$(TARGET_ELF):$(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@ 

%.o:%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

%.o:%.s
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

%.o:%.S
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

mksunxiboot:
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ tools/mksunxiboot.c

clean:
	-rm $(OBJS) $(TARGET)