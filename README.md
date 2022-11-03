# RoverOS
### About
This is an update from pure grub to an EFI bootloader.  
It is likely to have bugs, oversights, and is likely to not work on all hardware.  
### Planned features
These are things I will likely add in the future
* 2MB Paging
* Multitasking
* Networking
* Custom libc
* Auto makefile config
* ARM support
* Proper UI implementation over legacy x86 RoverOS

### Requierments
* uefi-run
* 'gcc' or a cross compiler
* ovmf
* Make
* mtools
* xorriso
* 'gnu-efi' This will have to be compiled from source and installed from the package manager for headers
* 'qemu' x86_64 version

### Compiling
1. cd to RoverOS directory base
2. Compile gnu-efi from source
3. set 'GNU_EFI_PATH' in the makefile to the absolute path of your compiled gnu-efi with no '' or ""  
Example: /home/rover/Documents/gnu-efi
4. set 'CC' in the makefile to your desired GCC compiler (cross compiler recommended)
5. run 'make required' to copy required gnu-efi files from 'GNU_EFI_PATH'
6. run 'make hdd' to create a fat32 .img
7. run 'make' to compile the .efi application  

### Using on real hardware (sudo required)
I use Ubuntu with GRUB, so this may be different for other distros.  
Windows will be completely different and this Makefile will not work for it.
1. Create /RoverOS/Boot in the EFI directory of your efi system partition
Example: /boot/efi/EFI/RoverOS/Boot
2. set 'ESP_PATH' to the /RoverOS/Boot folder in EFI system partition
Example: /boot/efi/EFI/RoverOS/Boot
3. set 'USE_ESP' to 1
4. run 'make' or 'make esp' if the efi app is already compiled
5. Put the following in your 40_custom grub file. Be sure to edit it to the EFI file in your ESP path
```
menuentry "RoverOS" {
        set root='hd0,gpt1'
        chainloader /EFI/RoverOS/Boot/RoverOS.efi
}
```
This file is in /etc/grub.d/ for Ubuntu
6. update grub config via command (Search up how to do it for your distro)
7. Reboot and the option 'RoverOS' should be there

### Unsupported hardware
* Non UEFI systems
* CSM only UEFI systems
* Non x64 CPUs
* ARM CPUs