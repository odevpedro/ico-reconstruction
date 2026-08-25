	.text
	.p2align 3
	.globl	pobj_hB
	.ent	pobj_hB
pobj_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	j	0x102858
	nop	
	j	0x10ecb8
	nop	
	jr	$31
	move	$2,$0
	.set	macro
	.set	reorder
	.end	pobj_hB
