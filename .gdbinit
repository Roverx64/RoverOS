target remote localhost:1234
add-symbol-file ./Other/sym/Fortuna.sym
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
define f
i frame $arg0
end
layout asm