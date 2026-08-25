	.text
	.p2align 3
	.globl	enemy1_hD
	.ent	enemy1_hD
enemy1_hD:
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
	.end	enemy1_hD
