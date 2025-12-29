// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Gemini-AI
 *
 * 这是一个简单的Linux字符设备驱动程序，用于教学目的。
 * 遵循现代内核接口规范。
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>      // 包含了cdev结构体和相关函数
#include <linux/device.h>    // 包含了class和device创建等Udev相关函数
#include <linux/uaccess.h>   // 包含了copy_to_user/copy_from_user等函数

#define DEVICE_NAME "hello_char" // 设备名，将出现在/dev/DEVICE_NAME
#define CLASS_NAME  "hello_class" // 设备类名，将出现在/sys/class/CLASS_NAME

MODULE_LICENSE("GPL");
MODULE_AUTHOR("caishd"); // 请替换为您的名字
MODULE_DESCRIPTION("一个适配RPi3的简单字符驱动模型");
MODULE_VERSION("1.0");

// --- 全局变量定义 ---
static dev_t dev_num;               // 用于存储动态分配的设备号 (主次设备号)
static struct cdev hello_cdev;      // 内核中代表字符设备的结构体
static struct class *hello_class;   // 设备类结构体指针
static struct device *hello_device; // 设备实例结构体指针

#define MSG_BUFFER_LEN 256                          // 内核缓冲区大小
static char message_buffer[MSG_BUFFER_LEN] = {0};   // 用于存储从用户空间接收到的数据
static int message_len = 0;                         // 缓冲区中实际数据长度

// --- file_operations 函数声明 ---
static int     dev_open(struct inode *inodep, struct file *filep);
static int     dev_release(struct inode *inodep, struct file *filep);
static ssize_t dev_read(struct file *filep, char __user *user_buffer, size_t len, loff_t *offset);
static ssize_t dev_write(struct file *filep, const char __user *user_buffer, size_t len, loff_t *offset);

// file_operations 结构体实例
static const struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .release = dev_release,
    .read    = dev_read,
    .write   = dev_write,
};

// --- 模块初始化函数 ---
static int __init hello_char_init(void)
{
    int ret;

    // 1. 动态分配一个字符设备号区域
    // alloc_chrdev_region(dev_t* dev, unsigned int firstminor, unsigned int count, const char* name)
    // dev: 用来存储分配到的设备号
    // firstminor: 起始次设备号，通常为0
    // count: 需要分配的设备号数量
    // name: 设备名
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("无法分配设备号\n");
        return ret;
    }
    pr_info("设备号分配成功. 主设备号=%d, 次设备号=%d\n", MAJOR(dev_num), MINOR(dev_num));

    // 2. 初始化 cdev 结构体，并与 file_operations 关联
    cdev_init(&hello_cdev, &fops);
    hello_cdev.owner = THIS_MODULE;

    // 3. 将 cdev 添加到内核，使其生效
    // cdev_add(struct cdev* cdev, dev_t dev, unsigned int count)
    ret = cdev_add(&hello_cdev, dev_num, 1);
    if (ret < 0) {
        pr_err("cdev 添加失败\n");
        // 如果添加失败，必须释放之前分配的设备号
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    // 4. 创建设备类 (class)，为自动创建设备节点做准备
    // class_create(owner, name)
    hello_class = class_create(CLASS_NAME);
    if (IS_ERR(hello_class)) {
        pr_err("创建设备类失败\n");
        ret = PTR_ERR(hello_class);
        goto cleanup_cdev;
    }
    pr_info("设备类创建成功\n");

    // 5. 在刚创建的类下创建设备实例 (device)，udev会自动在/dev下创建设备文件
    // device_create(class, parent, devt, drvdata, fmt, ...)
    hello_device = device_create(hello_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(hello_device)) {
        pr_err("创建设备失败\n");
        ret = PTR_ERR(hello_device);
        goto cleanup_class;
    }
    pr_info("设备创建成功，位于 /dev/%s\n", DEVICE_NAME);

    // 初始化内核消息
    strcpy(message_buffer, "Hello from kernel!\n");
    message_len = strlen(message_buffer);

    return 0;

// --- 错误处理: 使用goto进行优雅的资源释放 ---
cleanup_class:
    class_destroy(hello_class);
cleanup_cdev:
    cdev_del(&hello_cdev);
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

// --- 模块退出函数 ---
static void __exit hello_char_exit(void)
{
    // 资源释放的顺序与申请顺序相反 (LIFO)
    device_destroy(hello_class, dev_num); // 销毁设备
    class_destroy(hello_class);           // 销毁设备类
    cdev_del(&hello_cdev);                // 从内核移除cdev
    unregister_chrdev_region(dev_num, 1); // 释放设备号

    pr_info("hello_char 驱动已卸载\n");
}

// --- file_operations 函数实现 ---

// 打开设备文件时被调用
static int dev_open(struct inode *inodep, struct file *filep)
{
    pr_info("设备已被打开\n");
    return 0;
}

// 关闭设备文件时被调用
static int dev_release(struct inode *inodep, struct file *filep)
{
    pr_info("设备已被关闭\n");
    return 0;
}

// 从设备读取数据时被调用
static ssize_t dev_read(struct file *filep, char __user *user_buffer, size_t len, loff_t *offset)
{
    int error_count;
    ssize_t bytes_to_read;

    // 如果offset已经大于或等于消息长度，说明用户已经读完，返回0 (EOF)
    if (*offset >= message_len) {
        return 0;
    }

    // 计算本次还能读取多少字节
    bytes_to_read = message_len - *offset;
    if (bytes_to_read > len) {
        bytes_to_read = len;
    }

    // 使用 copy_to_user 将内核空间的数据安全地复制到用户空间
    // 返回0表示成功，返回>0的值表示有多少字节未能复制
    error_count = copy_to_user(user_buffer, message_buffer + *offset, bytes_to_read);
    if (error_count != 0) {
        pr_err("发送数据给用户时出错\n");
        return -EFAULT;
    }

    // 更新文件偏移量
    *offset += bytes_to_read;

    pr_info("已发送 %zd 字节数据给用户\n", bytes_to_read);
    return bytes_to_read; // 返回实际读取的字节数
}

// 向设备写入数据时被调用
static ssize_t dev_write(struct file *filep, const char __user *user_buffer, size_t len, loff_t *offset)
{
    // 检查写入数据长度是否超过缓冲区大小
    if (len >= MSG_BUFFER_LEN) {
        pr_warn("写入的数据过长，已截断\n");
        len = MSG_BUFFER_LEN - 1;
    }

    // 使用 copy_from_user 将用户空间的数据安全地复制到内核空间
    if (copy_from_user(message_buffer, user_buffer, len) != 0) {
        pr_err("从用户接收数据时出错\n");
        return -EFAULT;
    }

    // 确保字符串以null结尾
    message_buffer[len] = '\0';
    message_len = len;

    pr_info("从用户接收到 %zu 字节数据: %s\n", len, message_buffer);

    return len; // 返回实际写入的字节数
}

// --- 模块注册 ---
module_init(hello_char_init);
module_exit(hello_char_exit);