TARGET = gpio_driver

obj-m := $(TARGET).o

$(TARGET).ko: $(TARGET).c
	make -C /lib/modules/`uname -r`/build M=`pwd` modules

clean:
	make -C /lib/modules/`uname -r`/build M=`pwd` clean
	

