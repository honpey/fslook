obj-m = fslook.o

#transport_ring_buffer.o
fslook-objs := main.o dentry.o super.o debug.o transport_channel.o
KVER:=$(shell uname -r)
KVER=4.4.0-21-generic
CURRENT_PATH:=$(shell pwd)
all:
	make -C /lib/modules/$(KVER)/build M=$(CURRENT_PATH) modules
clean:
	rm *.ko
	rm *.o
	rm *.mod.c
	rm *.order
	rm Module.symvers
