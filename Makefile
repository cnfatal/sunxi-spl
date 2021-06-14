CROSS_COMPILE = arm-none-eabi-

CC			= $(CROSS_COMPILE)gcc
LD			= $(CROSS_COMPILE)ld
OBJCOPY		= $(CROSS_COMPILE)objcopy

CFLAGS		= 
LDFLAGS     = -emain --script=spl.lds --trace --verbose=5
OBJCOPYFLAGS= --strip-all -O binary

LIBGCC := -L$(shell dirname `$(CC) $(CFLAGS) -print-libgcc-file-name`) -static -lgcc
LIBS += $(LIBGCC)
INCLUDE += -Iinclude
SRC += src

LDFLAGS += $(LIBS)

srccs += $(foreach dir,$(SRC),$(wildcard $(dir)/*.c))
srcss += $(foreach dir,$(SRC),$(wildcard $(dir)/*.s))
srcSs += $(foreach dir,$(SRC),$(wildcard $(dir)/*.S))
OBJS  = $(srccs:.c=.o) $(srcss:.s=.o) $(srcSs:.S=.o)

TARGET = spl.bin
TARGET_NOELF = $(patsubst %.bin,%-noelf.bin,$(TARGET))
TARGET_BOOT = $(patsubst %.bin,%-boot.bin,$(TARGET))

mksunxiboot = tools/mkboot

all:$(TARGET_BOOT)

$(TARGET_BOOT):$(TARGET_NOELF) $(mksunxiboot)
	$(mksunxiboot) $< $@

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
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $@.c
	chmod +x $(mksunxiboot)

clean:
	-rm $(OBJS) *.bin  $(mksunxiboot)