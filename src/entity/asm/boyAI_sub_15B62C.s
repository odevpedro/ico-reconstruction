	.text
	.p2align 3
	.globl	boyAI_sub_15B62C
	.ent	boyAI_sub_15B62C
boyAI_sub_15B62C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$7,-0x6e0c($28)
	lw	$2,0xd0($3)
	ori	$2,$2,1
	.word	0x14c70004
	sw	$2,0xd0($3)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15B62C
