	.text
	.p2align 3
	.globl	chandelier_hC
	.ent	chandelier_hC
chandelier_hC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jr	$31
	move	$2,$0
	.set	macro
	.set	reorder
	.end	chandelier_hC
