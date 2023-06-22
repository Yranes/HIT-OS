## 2 保护模式和内存管理

### 2.1 内存管理概览

#### 分段和分页

IA-32体系结构的内存管理被分为两个部分：分段和分页。分段提供了一种隔离单个代码、数据和堆栈模块的机制，以便多个程序(或任务)可以在同一处理器上运行，而不会相互干扰。分页为实现传统的按需分页的虚拟内存系统提供了一种机制，在虚拟内存系统中，程序执行环境的各个部分根据需要映射到物理内存中。分页还可以用于在多个任务之间提供隔离。

当在保护模式下操作时，必须使用某种形式的分段。但是，分页的使用是可选的。这两种机制(分段和分页)可以配置为支持简单的单程序(或单请求)系统、多任务系统或使用共享内存的多处理器系统。

#### 逻辑地址

逻辑地址由段选择子和偏移量组成。段选择子是段的唯一标识符。除此之外，它还将一个偏移量提供给一个称为段描述符的数据结构的描述符表(例如全局描述符表GDT)。逻辑地址的偏移部分被添加到段的基本地址中，以便在段中定位一个字节。

#### 线性地址

处理器的可寻址内存空间被称为线性地址空间。线性地址是处理器线性地址空间中的32位地址。线性地址是由逻辑地址通过段基址加法变换得到的地址。在IA-32系统中，通过分页机制将线性地址映射到物理地址。线性地址是操作系统看到的地址，它对应于进程的虚拟地址空间。

#### 物理地址

物理地址空间被定义为处理器可以在其地址总线上生成的地址范围。物理地址是线性地址通过分页机制映射到物理地址的地址。在IA-32系统中，分页机制将线性地址分成大小相等的页，每个页被映射到一个物理页框，物理页框的地址由页目录和页表确定。物理地址是CPU最终访问的地址，它对应于实际的硬件地址。

#### 线性地址到物理地址的转换

如果不使用分页，处理器的线性地址空间将直接映射到处理器的物理地址空间。

由于线性地址空间往往比物理内存中一次性容纳所有的空间要大得多，因此需要某种“虚拟化”线性地址空间的方法。这种方法就是分页。

分页使用少量物理内存(RAM和ROM)和一些磁盘存储来模拟大型线性地址空间。在使用分页时，每个段被划分为页(通常每个大小为4 kb)，这些页可以存储在物理内存中，也可以存储在磁盘上，并由操作系统维护页表索引和页表。当程序试图访问线性地址空间中的地址位置时，处理器使用页索引和页表将线性地址转换为物理地址，然后在内存位置上执行请求的操作。如果当前访问的页面不在物理内存中，处理器将中断程序的执行(通过生成一个页面错误异常)。然后，操作系统或执行程序将页面从磁盘读入物理内存，并继续执行程序。

#### 逻辑地址到线性地址的转换

将会在2.3节介绍

### 2.2 分段机制

IA-32架构支持的分段机制可用于实现各种各样的系统设计。这些设计的范围很广，从只使用最小程度的分段来保护程序的平面模型，到使用分段来创建一个健壮的运行环境的多分段模型，在这个环境中可以可靠地执行多个程序和任务。

#### 基本平面模型

操作系统和应用程序可以访问连续的、未分段的地址空间。这种基本的平面模型尽可能地向系统设计人员和应用程序编程人员隐藏了体系结构的分割机制。

使用IA-32架构实现基本的平面内存模型，必须至少创建两个段描述符，一个用于引用代码段，另一个用于引用数据段。这两个段都映射到整个线性地址空间，即：两个段描述符具有相同的基地址值0和相同的段限制(4 GBytes)。通过将段限制设置为 4 GBytes，即使没有物理内存驻留在特定地址，分段机制也不会为超出限制的内存引用生成异常。

ROM(EPROM)通常位于物理地址空间的顶部，因为处理器从FFFF_FFF0H开始执行。RAM (DRAM)被放在地址空间的底部，因为重置初始化后DS数据段的初始基址为0。

![3-2](./images/feagure%203-2.png "Feagure 3-2")

#### 受保护的平面模型

受保护的平面模型与基本的平面模型类似，只是将段限制设置为物理内存实际存在的地址范围。然后，在试图访问不存在的内存时，将生成一个通用保护异常(#GP)。该模型提供了针对某些程序bug的最低程度的硬件保护。

这个受保护的平面模型可以变得更多的复杂性以提供更多的保护。这种平面分段模型和简单的分页结构可以保护操作系统不受应用程序的影响，并且通过为每个任务或进程添加单独的分页结构，还可以保护应用程序不受其他应用程序的影响。

![3-3](./images/feagure%203-3.png "Feagure 3-3")

#### 多分段模型

多段模型使用分段机制来提供对代码、数据结构、程序和任务的全面硬件强制保护。在这里，每个程序(或任务)都有自己的段描述符表和自己的段。段可以完全对其分配的程序私有，也可以在程序之间共享。对所有段的访问以及对系统上运行的各个程序的执行环境的访问由硬件控制。

访问检查不仅可以阻止引用在段的限制之外的地址，还可以阻止在某些段中执行不允许的操作。例如，如果代码段被指定为只读段，可以使用硬件来防止写入代码段。为段创建的访问权限信息也可用于设置保护环或级别。保护级别可用于保护操作系统过程免受应用程序的未授权访问。

![3-4](./images/feagure%203-4.png "Feagure 3-4")

### 2.3 逻辑地址和线性地址的转换

#### 段选择子

段选择子是一个段的16位标识符，它并不直接指向段，而是指向目标段的段描述符。包含以下内容：

- 索引（3-15 bit），从GDT或LDT中的8192个描述符中选择一个。处理器将索引值乘以8(段描述符中的字节数)，并将结果加上到GDT或LDT的基地址。
- TI flag（2 bit），指定要使用的描述符表，置0此标志将选择GDT，置1将选择当前LDT。
- RPL（0-1 bit），请求的特权级别。指定选择子的特权级别。特权级别的范围从0到3，0是最特权的级别。

#### 段寄存器

为了减少地址转换时间和编码复杂性，处理器提供最多可容纳6个段选择子的寄存器。每个段寄存器支持一种特定的内存引用(代码、堆栈或数据)。对于几乎任何类型的程序执行，至少代码段(CS)、数据段(DS)和堆栈段(SS)寄存器必须加载有效的段选择子。处理器还提供了三个额外的数据段寄存器(ES、FS和GS)，这些寄存器可用于为当前正在执行的程序(或任务)提供额外的数据段。

段寄存器有两种加载指令:

- 直接加载指令，如MOV、POP、LDS、LES、LSS、LGS和LFS指令。这些指令显式地引用段寄存器。
- 隐式的加载指令，例如调用远指针、JMP和RET指令、SYSENTER和SYSEXIT指令，以及IRET、INTn、INTO和INT3指令。这些指令改变CS寄存器(有时是其他段寄存器)的内容，作为操作的附带部分。

#### 段描述子

段描述子是GDT或LDT中的数据结构，它向处理器提供段的大小和位置，以及访问控制和状态信息。

段描述子中的标志和字段如下：

![3-8](./images/feagure%203-8.png "Feagure 3-8")

- 段限制域：指定段的大小限制。
- 基地址字段：段在线性地址空间中的基地址。处理器将三个基本地址字段放在一起，形成一个32位的值。段基址应该对齐到16字节边界。
- 类型字段：指示段或门类型，并指定可以对段进行的访问类型和增长方向。
- S标志：指定段描述符是针对系统段（0），还是针对代码段或数据段（1）。
- 描述符特权级别字段：指定段的特权级别。特权级别的范围从0到3，0是最高级别。
- 段存在标志：指示段是否存在在内存中。
- D/B标志：根据段描述符是可执行代码段、展开数据段还是堆栈段执行不同的功能。(对于32位代码和数据段，这个标志应该总是设置为1，对于16位代码和数据段，应该设置为0。)
- G标志：确定段极限字段的比例。当粒度标志清除时，段限制以字节单位解释;当设置标志时，段限制以4-KByte单位解释。
- L标志：在IA-32e模式下，段描述符的第二个双字的第21位表示一个代码段是否包含本地64位代码。值1表示在64位模式下执行此代码段中的指令。值0表示在兼容模式下执行此代码段中的指令。如果设置了L位，则必须清除D位。当不在IA-32e模式下或对于非代码段，第21位是保留的，应该始终设置为0。
- 可用和保留的位：段描述符的第二个双字的第20位可由系统软件使用。

#### 逻辑地址和线性地址的转换

逻辑地址由16位段选择子和32位偏移量组成。段选择子标识字节所在的段，偏移量指定字节在段中的相对于段的基地址的位置。

![3-5](./images/feagure%203-5.png "Feagure 3-5")

把一个逻辑地址转换成一个线性地址，处理器做以下工作:

- 使用段选择子中的偏移量来定位GDT或LDT中该段的段描述子，并将其读入处理器。
- 检查段描述子以检查段的访问权限和范围，以确保段是可访问的，并且偏移量在段的范围内。
- 将段描述子中的段的基址添加到偏移量中以形成线性地址。

### 2.4 描述符的分类

#### 数据段描述符和代码段描述符

当S位置1时，段为数据或代码段，数据段的三个低阶位（8，9，10）决定了其是否可读写、是否具有可扩展属性，可否访问。代码段的三个低阶位（8，9，10）决定了其是否可执行，可访问，是否满足一致性。

![t3-5](./images/table%203-1.png)

数据段描述符第11位置0，代码段描述符第11位置1。

#### 系统描述符

当S标志置0时，表示其为系统描述符，有以下几类：

- 局部描述符表描述符，LDT位于LDT类型的系统段中。GDT必须包含LDT段的段描述符。如果系统支持多个LDT，则每个LDT在GDT中必须有单独的段选择器和段描述符。
- 任务状态段描述符，TSS描述符只能放在GDT中，它们不能放在LDT或IDT中。基本字段、限制字段和DPL字段以及粒度和当前标志的功能与它们在数据管理描述符中的使用类似，在大多数系统中，TSS描述符的DPLs设置为小于3的值，因此只有特权软件才能执行任务切换。但是，在多任务应用程序中，某些TSS描述符的DPLs可能被设置为3，以允许在应用程序(或用户)特权级别上进行任务切换。
- 调用门描述符，调用门用于在不同特权级之间实现受控的程序控制转移，通常仅用于使用特权级保护机制的操作系统中。本质上，它只是一个描述符，一个不同于代码段和数据段的描述符，可以安装在GDT或者LGT中，但是不能安装在IDT（中断描述符表）中。
- 中断门描述符，包含中断处理程序所在的段选择子和段内偏移地址，当通过此方式进入中断后，标志寄存器eflags中的IF位自动置0，表示把中断关闭，避免中断嵌套。中断们只存在于中断描述符表IDT。
- 陷阱门描述符，通过陷阱门进入中断，标志寄存器eflags的IF位不会自动置0，陷阱门只允许存在于IDT中。
- 任务门描述符，需要和任务状态段（TSS）配合使用，这是Intel处理器在硬件一级提供的任务切换机制。任务门可以存在于全局描述符GDT、局部描述符表LDT以及中断描述符表IDT中。