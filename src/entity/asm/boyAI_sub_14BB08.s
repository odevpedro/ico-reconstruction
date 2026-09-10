	.text
	.p2align 3
	.globl	boyAI_sub_14BB08
	.ent	boyAI_sub_14BB08
boyAI_sub_14BB08:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mtc1	$1,$f0
	lw	$3,0x670($2)
	swc1	$f0,0x58($3)
	jr	$31
	sw	$0,0x54($3)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14BB08
