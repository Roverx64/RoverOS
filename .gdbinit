target remote localhost:1234
add-symbol-file ./Other/Fortuna.sym
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
define stack
i s
end
set pagination off
layout asm