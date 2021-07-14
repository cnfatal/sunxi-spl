CROSS_COMPILE = arm-none-eabi-

HOSTCC		= gcc

CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
OBJCOPY		= $(CROSS_COMPILE)objcopy

CFLAGS		= --debug
LDFLAGS     = -emain --trace --verbose=5
OBJCOPYFLAGS= -O binary --gap-fill=0xff

LIBGCC := -L$(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -static -lgcc
LIBS += $(LIBGCC)
INCLUDE += -Iinclude
SRC += src

LDFLAGS += $(LIBS)

srccs += $(foreach dir,$(SRC),$(wildcard $(dir)/*.c))
srcss += $(foreach dir,$(SRC),$(wildcard $(dir)/*.s))
srcSs += $(foreach dir,$(SRC),$(wildcard $(dir)/*.S))
# OBJS  = $(srccs:.c=.o) $(srcss:.s=.o) $(srcSs:.S=.o)

TARGET = spl.bin
# TARGET = uart0-helloworld-sdboot.bin
TARGET_NOELF = $(patsubst %.bin,%-noelf.bin,$(TARGET))
TARGET_BOOT = $(patsubst %.bin,%-boot.bin,$(TARGET))
LDS = $(patsubst %.bin,%.lds,$(TARGET))
OBJS = src/$(patsubst %.bin,%.o,$(TARGET))

LDFLAGS += --script=$(LDS)
# LDFLAGS += --script=spl.lds

mksunxiboot = tools/mksunxiboot

all:$(TARGET_BOOT)

$(TARGET_BOOT):$(TARGET_NOELF) $(mksunxiboot)
	$(mksunxiboot) $< $@
	dd if=/dev/zero of=boot.bin bs=8k count=1
	cat $@ >> boot.bin

$(TARGET_NOELF):$(TARGET)
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

$(TARGET):$(OBJS)
	$(LD) $^ $(LDFLAGS) -o $@ 

%.o:%.c
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

%.o:%.s
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

%.o:%.S
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<

$(mksunxiboot):
	$(HOSTCC) $(CFLAGS) $(INCLUDE) -o $@ $@.c
	chmod +x $(mksunxiboot)

clean:
	-rm $(OBJS) *.bin  $(mksunxiboot)