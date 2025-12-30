# Linux驱动学习进度

## 当前目标
- [x] 搭建基于 VMware 和 Ubuntu 的嵌入式交叉编译环境。
- [x] 为 Raspberry Pi 3 (BCM2837/BCM2710) 编译自定义 Linux 内核。
- [x] 将自定义内核成功烧写到 SD 卡并启动树莓派。
- [x] 验证自定义内核已成功运行。
- [x] 完成字符设备驱动开发的完整实践闭环。
- [ ] 开始学习平台设备驱动模型 (Platform Device Driver Model)。

## 已完成任务
- [x] 放弃了WSL2方案，因为无法直接访问SD卡读卡器。
- [x] 在 VMware Ubuntu 虚拟机中成功配置了交叉编译工具链。
- [x] 成功编译了 Linux 内核版本 `6.12.62-v7+`。
- [x] 识别出正确的设备树文件为 `bcm2710-rpi-3-b.dtb`。
- [x] 将编译好的 `zImage`, `modules`, `dtb`, 和 `overlays` 成功复制到 SD 卡。
- [x] 使用 `uname -a` 命令成功验证新内核 (`6.12.62-v7+ #1 SMP Mon Dec 29 15:34:46 JST 2025`) 已在树莓派上运行。
- [x] 编写了第一个字符设备驱动程序 `hello_char.c`，并理解了其核心注册与注销流程。
- [x] 学习并实践了用于交叉编译内核模块的 `Makefile` 编写。
- [x] 配置了 `.gitignore` 以忽略编译产物，保持版本库整洁。
- [x] 成功编译驱动生成 `hello_char.ko` 内核模块。
- [x] 整理并撰写了 `003_第一个字符驱动程序分析.md` 学习笔记。
- [x] 将 `hello_char.ko` 模块成功部署到树莓派并使用 `insmod`/`rmmod` 加载卸载。
- [x] 通过读写 `/dev/hello_char` 设备节点来测试驱动的功能。
- [x] 学习并掌握了使用 `pr_info` / `dmesg` 进行内核调试的基本方法。
- [x] 解决了 `Permission denied` 问题，通过实现 `class->devnode` 回调，掌握了在驱动层设置设备文件权限的方法。
- [x] 修复了因 `const` 限定符缺失导致的编译错误，加深了对内核API严谨性的理解。
- [x] 解决了 Git 认证问题，掌握了使用个人访问令牌(PAT)进行 `git push` 的方法。
- [x] 撰写了 `004_字符设备驱动的权限管理.md` 学习笔记。

## 下一步计划
- [ ] **理论学习**: 阅读《嵌入式Linux设备驱动程序开发指南》**第5章：平台设备驱动 (Platform Device Drivers)**。
- [ ] **目标理解**: 理解为什么需要平台设备驱动模型（处理非热插拔、挂载在SoC总线上的设备）。
- [ ] **核心概念**: 学习 `platform_driver` 和 `platform_device` 两个核心结构体，以及它们如何通过 `probe` 函数进行匹配和初始化。
- [ ] **实践准备**: 准备编写第一个平台驱动，并为其撰写相应的设备树（Device Tree）节点。
