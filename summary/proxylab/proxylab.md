# CSAPP Learning

---
*This document is specially for ProxyLab of book CSAPP.*

## 什么是代理？
代理（Proxy）是网络连接中间的**翻译官**，它既是一个服务端，也是一个客户端。

科学上网用的**梯子**，就是典型的网络代理。

## PartI 搭建基础

在Proxylab里面，我们的任务就是把用户发的HTTP 1.1请求**翻译**成HTTP 1.0，然后打包发给目标服务服务器。

HTTP是一个建立在**纯文本**基础上的高级协议，在Python里面集成过 `request` 包，可以**自动解析处理**，而在C里面，我们必须要**手动处理字符串**。

整个过程的流程：
* 和客户端建立链接
* 获取对应信息
* 打包封装字符串
* 发给目标服务器
* 从目标服务器获取资源
* 发还给客户端

代码实现是完全的**板子代码**。

### HTTP和板子代码的 getaddrinfo 是一个东西吗？

不是。后者是建立在**底层的socket上**。

## PartII 并发实现
我们通过**线程**给所有连接的客户端创建一个线程，然后处理任务，注意每个线程传入一个参数，即对应客户端的**文件描述符connfd**。

### 传参姿势与预防竞争条件
可能有很多个线程同时进行，我们**不可能**把connfd存在一个变量里，但是就算分开赋值也会**出现问题**：我们没法确定**赋值**和**获取到新的connfd值**谁更先进行。

所以我们要在 `main` 里面先 `malloc` 堆区地址给 `connfd` 存储，给线程传入指针，然后该线程结束时**线程内部** `free` 掉区块。

## PartIII  Cache
由于代理访问服务器的**时间需求很长**，为了方便访问，我们需要在Proxy上**做缓存**，存一些相关的文件数据，这样可以直接提供**一部分**以提高效率。

```C
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAXLINE 8192

static int cache_used = 0;

struct Cache{
    char url[MAXLINE];
    int history;
    char content[MAX_OBJECT_SIZE];
    int size;
} cache[8];
```

我们根据要求**手动**创建一个8长度的 `cache` **结构体数组**，这样避免手动 `malloc` 挤占堆区且不方便管理。

然后定义
```C
int cache_load(char url[MAXLINE]);
void cache_save(char url[MAXLINE], int size, char content[MAX_OBJECT_SIZE]);
```

这两个方法。

**注意C的细节问题**：字符串最后一位以 `\0` 终止，但我们读写（二进制）文件的时候，由于文件编码可能中间出现 `\0`，所以**读写字符串和二进制bytes**的方法不同。

其余实现方法同Cachelab。

### 锁怎么用？
这里涉及到**公共变量的读写**，我们必须要调用锁，但是只有在**最关键的**读写公共变量前后才上锁/解锁，避免堵塞其他线程。

---

***By Tab_1bit0***