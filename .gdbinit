target remote localhost:1234
symbol-file ./Other/RoverOS.sym
define rr
i r cr0 cr3 cr4 efer rsp rip
end
define vara
i address $arg0
end
define var
p/x $arg0
end
define pk
x/gx $cr3
x/gx $rdi
end
define x64
x/gx $arg0
end
set pagination off
layout asm
break initACPI
continue