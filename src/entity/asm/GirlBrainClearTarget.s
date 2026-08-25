	.text
	.p2align 3
	.globl	GirlBrainClearTarget
	.ent	GirlBrainClearTarget
GirlBrainClearTarget:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$4,0x29
	j	0x182890
	addiu	$4,$4,-0x5770
	nop	
	.set	macro
	.set	reorder
	.end	GirlBrainClearTarget
