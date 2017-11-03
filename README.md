# bitvisor-gdb
gdbserver implementation on **Type1 Thin Hypervisor(BitVisor)**.
You can debug Guest OS on BitVisor from your **gdb** on local machine.

# Install
Firstly you need to download **BitVisor**. Plz read [official document](https://www.bitvisor.org/)  
To enable gdb remote debugging function, please edit defconfig as following.  
```c:defconfig
struct config_data config = {
        .ip  = {
               .ipaddr = { 192, 168, 0, 5 },
               .netmask = { 255, 255, 255, 0 },
               .gateway = { 192, 168, 0, 1 },
        },
        .driver = {
                .pci = "driver=pro1000,net=ip,tty=1",
        },
```
If you wrote config as above, gdb server waits client connection at 192.168.0.5 on Intel PRO/1000 hardware.

# Usage
Then you can start system(ex. Linux kernel) debugging like:
```sh:
$ gdb -q
(gdb) target remote 192.168.0.5:1234
Warning: not running or target is remote
0xffffffff81428f41 in ?? ()
(gdb) hb *0xffffffff81055ab0     #Set breakpiont to do_page_fault()                                   
Hardware assisted breakpoint 1 at 0xffffffff81055ab0
(gdb) c
Continuing.
Breakpoint 1, 0xffffffff81055ab0 in ?? ()
(gdb) x/10i $rip
=> 0xffffffff81055ab0:  nop    DWORD PTR [rax+rax*1+0x0]
   0xffffffff81055ab5:  push   rbp
   0xffffffff81055ab6:  mov    rax,QWORD PTR gs:0xc440
   0xffffffff81055abf:  mov    rbp,rsp
   0xffffffff81055ac2:  push   r15
   0xffffffff81055ac4:  push   r14
   0xffffffff81055ac6:  push   r13
   0xffffffff81055ac8:  push   r12
   0xffffffff81055aca:  mov    r15,rdi
   0xffffffff81055acd:  push   rbx
(gdb) i r
rax            0x81784987       0x81784987
rbx            0xffffffff81784987       0xffffffff81784987
rcx            0x7f48fde0b9d0   0x7f48fde0b9d0
rdx            0x1      0x1
rsi            0xffff880087cb3e70       0xffff880087cb3e70
rdi            0xffff880087cb3e78       0xffff880087cb3e78
rbp            0x3      0x3
rsp            0xffff880087cb3e88       0xffff880087cb3e88
r8             0xffff880087cb0000       0xffff880087cb0000
r9             0x0      0x0
r10            0x3      0x3
r11            0x32683  0x32683
r12            0xffffffff818176a0       0xffffffff818176a0
r13            0xffff880034ca12c0       0xffff880034ca12c0
r14            0x3      0x3
r15            0xffff880034cca940       0xffff880034cca940
rip            0xffffffff81055ab0       0xffffffff81055ab0
eflags         0x83     [ CF SF ]
cs             0x10     0x10
ss             0x18     0x18
ds             0x0      0x0
es             0x0      0x0
fs             0x0      0x0
gs             0x0      0x0
```
NOTE: Port must be ```1234```.

# Reference
You can check the information about **Thin hypervisor**.

- What is thin hypervisor  
http://searchservervirtualization.techtarget.com/definition/thin-hypervisor
- シン・ハイパーバイザ2017 (前編) (JPN)  
https://ntddk.github.io/2017/03/04/thin-hypervisor-2017/


