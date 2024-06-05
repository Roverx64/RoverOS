target remote localhost:1234
add-symbol-file ./Other/Fortuna.sym
set pagination off
define vara
i address $arg0
end
define var
p/x $arg0
end
define x64
x/gx $arg0
end
define stack
i s
end
layout asm