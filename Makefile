KERN_DIR=/home/xiao/linux-3.5
PWD = $(shell pwd)
all:
	make -C $(KERN_DIR) M=$(PWD) modules

clean:
	make -C $(KERN_DIR) M=$(PWD) modules clean
	rm -rf modules.order
	
obj-m += bs81_dev.o
obj-m += bs81_drv.o
