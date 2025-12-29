# src/hello_char_driver/Makefile

# obj-m 表示我们将 hello_char.o 编译成一个可加载的内核模块
obj-m += hello_char.o

# KERNEL_DIR 是你之前编译内核的源码树路径
# 我们将使用它来构建我们的驱动模块
KERNEL_DIR = /home/caishd/rp3-dev/linux

# 'all' 是默认的 make 目标
all:
	make -C $(KERNEL_DIR) M=$(PWD) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules

# 'clean' 目标用于清理编译生成的文件
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean