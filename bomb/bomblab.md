# `debugger`
[[lldb#Assembly-level]]
# `objdump`
`objdump`可用于理解机器码
- `objdump -d`: 反汇编为汇编
- `objdump -S`: 反编译为源码，前提是编译时带有调试信息
- 查看ELF(Executable and Linkable Format)信息
	- `objdump -f`: 查看文件头
	- `objdump -h`: 查看section
	- `objdump -t`: 查看符号表
- `objdump -p`: 查看[[动态链接]]（`.plt`)
- `objdump -R`: 查看`.got`

机器码划分为功能不同的区域(section)
- `.text`：程序的机器码
- `.init`：程序初始化代码，在执行`main()`之前的初始化步骤
- `.plt`: Procedure Linkage Table, 用于动态链接的中间跳板
- `.got`: Global Offset Table，用于记录外部库的函数的地址
- `.data`：初始化的全局变量和静态变量
- `.rodata`：只读数据，例如字符串常量和 `const` 修饰的全局变量

`objdump -d`仅反汇编包含指令的section, 而不会反汇编包含数据的section, 因为指令和数据都是hexadecimal, 无法用同一机制反汇编
# `bomb.c`
`initialize_bomb()`  
然后是6个phase
```
input = read_line()
phase(input);
pase_defused()
```

## 函数`initialize_bomb`
call `signal(SIGINT, sig_handler)`  
`sig_handler`是一个自定义的函数，输出了一些字符

## 函数`phase_1`
call `strings_not_equal(input, 0x402400)`  
猜测若equal, 跳过`explode_bomb`  
试验`0x402400`失败

要仔细阅读这两个函数的逻辑有些困难，我可以在一定的基础上猜测其逻辑  
### 函数`strings_not_equal`
```
检查string_length是否相等
将参数解析为地址，检查地址上的字节序列是否相等
```
端点调试发现：输入`0x402400`，两者的string_length不等
### 函数`string_length`
```
遍历input的hexdecimal序列（在内存中hexadecimal的每一个字节对应一个唯一地址）
检查当前字节是否为0x0
若是，返回
若不是，访问的地址++,记录偏移量即字符长度至%eax
```

程序将输入视为字符，对比每一个字符的字节码是否与key的相等

```
string_length(0x402400) = 0x34
```

读取`0x402400`上的长度`0x34`的字符即可得到`phase_1`的key
```
(lldb) me read -c 0x34 0x402400 
```
注意在`lldb`的输出中`00`和`2e`都被解析为`.`，两者对应的符号不同

## 函数`phase_2`
### 函数`read_six_number`
参数`%rsi`的值是`%rsp`stack底部的地址`0x00007fffffffd870`  
若返回值不为1, explode
```
%rsp = 0x00007fffffffd850
%rdx = 0x00007fffffffd870  (%rdx) = 0xe8
%rcx = 0x00007fffffffd874  (%rcx) = 0xff
%rax = 0x00007fffffffd884  (%rax) = 0xff
no corresponding 'utf-8' char for these bytes

将%rax的值写入0x00007fffffffd858 （注意存储方式是以高地址向低地址顺序存入字节序列 Little-Endian）

%rax = 0x00007fffffffd880
将%rax写入0x00007fffffffd850
%r9 = 0x00007fffffffd87c
%r8 = 0x00007fffffffd878

此时出现的一个关键地址0x4025c3，写入%esi(%rsi的下32位)
%eax(%rax的下32位)设为0
int sscanf( const char *buffer, const char          *format, ... )
The `sscanf()` function reads data from a `char` array and writes it into memory locations specified by the arguments.
buffer是输入的字符input
format是"%d%d%d%d%d%d"6位数字
函数sscanf将input写入0x00007fffffffd870

整个函数的逻辑就是从input读取6个数字到以call这个函数时(%rsp)开始的6个参数上(%rdx) (%rcx) (%r8) (%r9) 此时寄存器不够用了，另外2个参数在内存0x00007fffffffd858和0x00007fffffffd850上（frame的Argument build area上）
input必须为6个hexadecimal整数，以空格分隔
6个整数位于[0x00007fffffffd870, 0x00007fffffffd884]区间上，每个整数占4字节
```

第一个整数需要以字节`01`结尾  
初始化变量`%rbp`,`%rbx`分别为内存`0x4(%rsp)`和`0x18(%rsp)`上的值  
`%rbp`相当于遍历由6个整数组成的数组的cursor
`%rbx`是input的6个整数存储地址区间的后一个地址，同时是一个sentinel用以终止遍历
### 循环
`%eax` = 内存`-0x4(%rbx)`上的值，input的一个整数
`%eax`翻倍  
测试`%eax == (%rbx)`, 若不相等，explode  
否则，更新：`%rbx += 0x4`，input的下一个数字的地址
测试`%rbp != %rbx`, 若相等，循环结束

```
遍历input的6个整数n[i]
if n[i-1]*2 != n[i]
	explode
```

所以key可以是`1 2 4 8 16 32`

## 函数`phase_3`
`%rcx`和`%rdx`是两个地址，是函数`sscanf`将input写入的地址 
```
rcx = 0x00007fffffffd89c
rdx = 0x00007fffffffd898
```
关键地址`0x4025cf`上的值是`%d %d`，故`%rdx`和`%rcx`上为整数

```
if M[rdx] > 0x7 then explode
eax = M[rdx]
goto M[0x402470]+8*rax=0x400f7c+d1*8 
```
这个indirect jump是一个switch结构的组成部分，根据前面的约束条件可推测一个有8个分支
```
eax = 0x137
if rcx == eax then phase_3 pass
otherwise explode
```
根据逻辑，构造key`1 311`即可

## 函数`phase_4`
phase前部分与phase_3相同
call `sscanf`将input以`%d %d`的形式写入以下两个地址上，input必须为2个整数
```
     rdx = 0x00007fffffffd898
     rcx = 0x00007fffffffd89c
```
```
if rdx > 0xe then explode
edi = M[rdx] // 输入的第一个整数
esi = 0x0
edx = 0xe

call func4
```

### `func4`
```
func4(d1 0x0, 0xe) // d1 = edi 常量
eax = edx
eax = eax - esi = edx - esi // 这里构造了两个数的差，在汇编层这里的语义就不明显
ecx = eax = edx - esi
ecx = ecx >> 31 // 取符号位
eax = eax + ecx
eax = eax >> 1 = eax / 2
ecx = eax + esi = (edx - esi) / 2 + esi

if ecx == d1 {
	return
} else if ecx < d1 {
	esi = rcx+0x1
	func4(d1, esi, edx)
	eax = eax*2+1
} else if ecx > d1 {
	edx = rcx-0x1
	func4(d1, esi, edx)
	eax = eax*2
}
```


这实际上是在**用二叉树遍历的方式编码搜索路径**：
`esi = low` `edx = high` `ecx = mid`
- 往左走：返回值 × 2
- 命中目标：返回 0
- 往右走：返回值 × 2 + 1

所以 `func4` 的返回值可以看作**路径的二进制表示**，从根到目标节点的每一步：左=0，右=1，最后命中时返回 0，再回溯组合。路径编码是从右往左解析的，比如(100)表示的路径是(左左右)

```
if eax != 0 then explode
if rcx != 0 then explode
phase_4 pass
```

输入的第二数必须为0  
因为返回值必须为0，所以不能右转，最后得到的数才能是0  
所以在中间值7左分支的节点的值都能通过  