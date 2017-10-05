#!/bin/sh
NAME=bitvisor
ELF=bitvisor.elf
EFI_PATH=/boot/efi/EFI/bitvisor
BOOT=boot/uefi-loader/loadvmm.efi
BOOT2=boot/uefi-loader/loadvmm.dll

if [ $1 = "remote" ]; then
  scp $ELF $BOOT $BOOT2 root@$2:$EFI_PATH
elif [ $1 = "reboot" ]; then
  ssh root@$2 reboot
else
  cp $ELF $EFI_PATH
  cp $BOOT $EFI_PATH
  cp $BOOT2 $EFI_PATH
fi
