# OS
Make a tiny OS.

### References
《X86从实模式到保护模式》

《汇编语言》王爽

《操作系统——精髓与设计原理》

《操作系统真象还原》

《跟我一起写Makefile》

《Linux命令行与Shell脚本编程大全》

《Linux/Unix系统编程手册》

### Logs
|    日期   |    事项   |
|   :----   |   :----   |
| 2021/3/18 | 编写并载入主引导程序MBR。 |
| 2021/3/19 | 编写内核加载器Loader，修改MBR并从硬盘读取内核加载器载入内存，从内核加载器中进入保护模式。 |
| 2021/3/20 | 增加内存检测功能，使用3种中断子功能探测内存容量。</br>完成二级页表的设置，成功开启分页机制。</br>花了2个小时弄懂了事先创建的页目录、页表与物理页的映射关系。不禁感叹，Linux的分页系统是真的巧妙。</br> 用C语言编写kernel.bin，并写入硬盘。|
| 2021/3/21 | 在loader中增加了mem_cpy字节拷贝、rd_disk_m_32硬盘读取和kernel_init内核初始化子过程，从硬盘读取kernel.bin并载入内存，接着根据elf文件头的信息，将kernel.bin中的segment载入内存中的正确位置。</br>学习特权级转换的相关知识。 |
| 2021/3/22 | 测试内核，发现gcc-m32...， ld -m elf_i386...编译链接的ELF文件中含有其他的段，这些段的虚拟地址没有分配物理块，因此在对ELF里面的段进行重定位的时候，访存错误导致了重启。</br>折腾了一天，猜测是gcc版本的问题。 |
| 2021/3/23 | 凌晨，将Ubuntu20.04.1、gcc9.3.0换成Ubuntu18.04.05、gcc7.5.0，之后一切正常，成功加载运行内核。|
| 2021/3/24 | 使用汇编实现put_char字符打印函数和put_str字符串打印函数，能够判断退格、换行以、回车符和空字符，测试成功。 |
| 2021/3/25 | 使用汇编实现put_int数字打印函数，给定32位数字，以16进制格式输出，输出时不输出前缀0，若数字全0则只输出0。 |
| 2021/3/26 | 学习中断相关的内容，内容过多，未上机实践。 |
| 2021/3/27 | 无 |
| 2021/3/28 | 费了九牛二虎之力又看了一遍中断的内容，然后画了N张示意图，终于看懂了源码，接着就是为内核添加中断功能，有空总结一下8259A可编程中断控制器的内容。 |
| 2021/3/29 | 学习内存管理相关内容，通过bitmap和内存池管理操作系统的内存，内存池分为内核池和用户池，另外，每个用户进程都有一个虚拟地址内存池，来管理用户动态分配的内存。 |
| 2021/3/30 | 学习内核线程的内容。 |
| 2021/3/31 | 添加8253定时器的初始化模块。 |
| 2021/4/01 | 修复增加定时器模块后出现General Protection Fault的bug，原因是kernel.S中那些会压入error_code的中断相应的中断处理程序，没有使用ERROR_CODE作为macro的第二个参数，导致这类错误会多压入0到栈中，从而发生错误。 |
| 2021/4/02 | 无 |
| 2021/4/03 | 实现了字符串操作函数memset、memcpy、memcmp、strcpy、strlen、strlen、strchr、strchr_r、strcat和str_cout，并进行了测试。初步开始写内存管理的位图bitmap。 |
| 2021/4/04 | 完成位图bitmap的数据结构设计与相关函数实现。测试的时候遇到了bug，结果发现是按位与 '&' 和 '!=' 的优先级问题，下次还真得注意一下，能加括号就加括号。 |

### Directories

| 目录    | 内容                               |
| ------- | ---------------------------------- |
| include | 主引导程序、内核加载器包含的头文件   |
| kernel  | 内核                              |
| tool    | 一些实用的小工具                   |
| boot    | 主引导程序和内核加载器              |
| hd      | 磁盘映像                          |
| configs | bochs配置文件                      |
| build | 目标文件和可执行文件 |
| test | 测试的相关文件归档 |

### Notes

#### 内联汇编

##### 基本内联汇编

格式为`asm [volatile] ("assembly code")`，带`[]`的是可选项。

`volatile`告诉编译器不要优化此处的代码。

`asm`与`__asm__`、`volatile`与`__volatile__`均是等效的。

`assembly code`的语法格式：

1. 指令必须被包含在双引号中，可以使用多个双引号对分开不同指令。
2. 指令之间需要使用分号，包括被不同对引号分开的指令。
3. 一对双引号不能跨行，如果要跨行必须在末尾使用`\`进行转义。

```assembly
asm("movl $9, %eax;""pushl %eax") ;√
asm("movl $9, %eax""pushl %eax") ;×
```

##### 扩展内联汇编

由于基本内联汇编功能较弱，且有可能会破坏系统正在使用的数据，因此有了扩展内联汇编，扩展内联汇编的就是在基本内联汇编的基础上，增加了输入、输出及其约束，我们可以使用指定C程序中的数据输入到汇编程序中，并令其输出到指定的位置。约束可以控制编译器对数据、输入输出的行为，以保证数据的正常。

基本格式：`asm [volatile] ("assembly code" : output : input : clobber/modify)`

大体格式与基本内联汇编相同，不同的是`output`、`input`和`clobber/modify`。

`output`: 用来指定汇编代码的数据如何输出给C代码用，这里每个操作数的格式是`"操作数修饰符约束名"(C变量名)`，修饰符通常为`=`，多个操作数之间使用逗号分隔。这一项可以忽略，但是冒号分隔符必须保留。

`input`: 与`output`类似，用于指定汇编代码的数据如何输出给C代码用，格式为`"[操作数修饰符]约束名"(C变量名)`，这里与`output`不同的是，操作数的修饰符是可选的，`[]`表示可选。

`clobber/modify`: 汇编代码执行后有可能会破坏一些寄存器或内存资源，通过这一项告知编译器，有哪些资源可能会遭到破坏，以便让编译器提前将资源保护起来。

###### 约束

寄存器约束（即要求编译器使用哪些寄存器）

- a: 表示寄存器eax/ax/al
- b: 表示寄存器ebx/bx/bl
- c: 表示寄存器ecx/cx/cl
- d: 表示寄存器edx/dx/dl
- D: 表示寄存器edi/di
- S: 表示寄存器esi/si
- q: 表示4个通用寄存器之一：eax/ebx/ecx/edx
- r: 表示6个通用寄存器之一：eax/ebx/ecx/edx/esi/edi
- g: 表示可以放到任意地点（寄存器和内存），即除了q之外还可以放到内存
- A: 把eax和edx组合成64位整数
- f: 表示浮点寄存器
- t: 表示第1个浮点寄存器
- u: 表示第2个浮点寄存器

###### 内存约束

- m: 表示操作数可以使用任意一种内存形式
- o: 操作数为内存变量

###### 立即数约束

- i
- F
- I
- J
- N
- O
- X

###### 通用约束

- 0~9