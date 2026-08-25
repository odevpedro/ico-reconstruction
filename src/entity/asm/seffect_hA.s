	.text
	.p2align 3
	.globl	seffect_hA
	.ent	seffect_hA
seffect_hA:
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
	.end	seffect_hA
