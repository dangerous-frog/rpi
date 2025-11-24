/* user.ld - Links user code at 0x0 where it actually runs */
ENTRY(user_process)

SECTIONS
{
    . = 0x0;  /* User space virtual address */
    
    .text.user : { build/user* (.text) build/term* (.text) build/user_printf* (.text)}
    .rodata.user : { build/user* (.rodata) build/term* (.rodata) build/user_printf* (.rodata)}
    .data.user : { build/user* (.data) build/term* (.data) build/user_printf* (.data)}
    .bss.user : { build/user* (.bss) build/term* (.bss) build/user_printf* (.bss)}
    .fake : {. = . + SIZEOF(.bss.user); } /* Fuckery, but hey, it works (bss is runtime, so ld doesn't care)!*/
    
    /* Debug sections - important for GDB */
    .debug_info     0 : { *(.debug_info) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_ranges   0 : { *(.debug_ranges) }
}