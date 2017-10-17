# bitvisor-gdb
gdbserver implementation on BitVisor.
You can debug Guest OS on BitVisor from your local machine.

# Install
Firstly you need to download BitVisor. Plz read [official document](https://www.bitvisor.org/)  
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
Then you can start debug like:
```sh:
$ gdb -q
target remote 192.168.0.5:1234
```
NOTE: Port must be ```1234```.
