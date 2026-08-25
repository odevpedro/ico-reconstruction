	.text
	.p2align 3
	.globl	girlBrain_sub_16C130
	.ent	girlBrain_sub_16C130
girlBrain_sub_16C130:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	.word	0x45000008
	move	$4,$16
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16C130
