	.text
	.p2align 3
	.globl	attackch63_hA
	.ent	attackch63_hA
attackch63_hA:
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
	.end	attackch63_hA
