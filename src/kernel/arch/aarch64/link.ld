SECTIONS
{
	. = 0xffff000000080000;
	.text.boot : { *(.text.boot) }
	. = ALIGN(0x00001000);
	user_begin = .;
	.text.user : { build/user* (.text) }
	.rodata.user : { build/user* (.rodata) }
	.data.user : { build/user* (.data) }
	.bss.user : { build/user* (.bss) }
	user_end = .;
	.text :  { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	. = ALIGN(0x8);
	bss_begin = .;
	.bss : { *(.bss*) } 
	bss_end = .;
	bss_size = bss_end - bss_begin;
	. = ALIGN(0x00001000);
	pg_dir = .;
	/* We need 1 PGD , 1 PUD and 4 PMD*/
	.data.pgd : { . += (6 * (1 << 12)); } 
	pg_dir_identity = .;
	.data.pgd_dir_identity : { . += (6 * (1 << 12)); }
}

