# RoverOS
### About
This is an update from pure grub to an EFI bootloader.  
It is likely to have bugs, oversights, and is likely to not work on all hardware.  
It is a hobby OS developed by one person.  
RoverOS is named after my dog, and that is where my username comes from as well.  

### Planned features
These are things I will likely add in the future
* 2MB Paging
* Multitasking
* Networking
* Custom libc
* Auto makefile config
* ARM support
* Proper UI implementation over legacy x86 RoverOS

### Unsupported hardware
* Non UEFI machines
* CSM only UEFI machines
* Non x64 (64 bit) CPUs
* ARM CPUs

### Requirements
* `uefi-run`
* `gcc` or an x86_64 cross compiler in /home/your_username/opt/cross/bin
* `ld`
* `ovmf` (I suggest looking up how to get ovmf for your distro)
* `Make`
* `mtools`
* `xorriso`
* `gnu-efi` (This will have to be compiled from source)
* `qemu` (x86_64 version)

### Compiling
#### Setting vars
Do not include the '' or "" unless specified. Avoid adding a space or / after a variable as Make will mess up  
**Correct:** `EXMAPLE_VAR = /path/to/something` **Incorrect:** `EXMAPLE_VAR = /path/to/something/ `  
* set `GNU_EFI_PATH` to the absolute path of the gnu-efi source directory  
Example: `GNU_EFI_PATH = /home/rover/Documents/gnu-efi`
* set `USE_ESP` to `1` only if copying the efi to your hdd
* set `USE_UEFI_RUN` to `1` (This will be optional when Qemu is set up on my end)
#### Gnu-efi
* download gnu-efi with `git clone https://git.code.sf.net/p/gnu-efi/code gnu-efi`
* cd and make
#### RoverOS
1. Ensure all vars are set as required above
2. cd to the base of the RoverOS folder (where the Makefile is)
3. run `make required` to copy all needed files from gnu-efi
4. run `make` to compile and run the EFI application

### Real hardware (Sudo required)
#### Creating folders
* cd into the `EFI` folder in your EFI System Partiton (ESP) (This is at `/boot/efi/EFI` on Ubuntu)
* create a folder named `RoverOS` && cd into it
* create a folder called `Boot`
#### Setting vars
* set `USE_ESP` to `1`
* set `ESP_PATH` to the `Boot` folder you created  
Example: `ESP_PATH = /boot/efi/EFI/RoverOS/Boot`
#### Compiling
* run `make` or `make esp` if RoverOS has already been compiled
#### GRUB
* Put the following in your `40_custom` grub file. Be sure to edit it to the EFI file in your ESP path
```
menuentry "RoverOS" {
        set root='hd0,gpt1'
        chainloader /EFI/RoverOS/Boot/RoverOS.efi
}
```
This file is at `/etc/grub.d/` for Ubuntu  
6. update grub config via command (Search up how to do it for your distro)  
7. Reboot and the option 'RoverOS' should be there