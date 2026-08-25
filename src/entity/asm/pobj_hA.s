	.text
	.p2align 3
	.globl	pobj_hA
	.ent	pobj_hA
pobj_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	j	0x10ecb8
	nop	
	jr	$31
	move	$2,$0
	.set	macro
	.set	reorder
	.end	pobj_hA
