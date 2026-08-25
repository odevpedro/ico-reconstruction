	.text
	.p2align 3
	.globl	girlBrain_sub_16F5C4
	.ent	girlBrain_sub_16F5C4
girlBrain_sub_16F5C4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	neg.s	$f1,$f2
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	.word	0x45010009
	move	$5,$16
	.word	0x1000000c
	addiu	$18,$18,1
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46001034
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16F5C4
