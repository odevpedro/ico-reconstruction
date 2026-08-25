	.text
	.p2align 3
	.globl	seffect_hB
	.ent	seffect_hB
seffect_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	seffect_hB
