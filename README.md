# vDB
一个key->value数据库，key和value均为string类型

Introduction
----
这是第一版，暂时所采用的数据结构是hash表,不支持windows环境

Environment
----
* OS:Ubuntu 18.04
* Compiler: g++ 7.4.0

## Build

	make

## Usage
	
	make会生成一个libv_db.a静态库
	接口声明在include目录
	测试代码在test目录
	源文件在src目录

## Test_Method

这里简单讲一下我自己的测试方式  

首先定义三种操作

1代表store操作

2代表delete操作

3代表fetch操作

然后根据参数生成了1w个数据

最后通过调用我的数据库接口与unorder_map对比结果来测试


## Implementation_Principle
具体实现原理请看https://blog.csdn.net/qq_34262582/article/details/104460853
