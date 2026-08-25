	.text
	.p2align 3
	.globl	girlBrain_sub_16E480
	.ent	girlBrain_sub_16E480
girlBrain_sub_16E480:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lwc1	$f1,0x3e8($16)
	add.s	$f0,$f0,$f3
	.word	0x46000834
	nop	
	.word	0x45000012
	lwc1	$f0,0x5c($29)
	lwc1	$f1,0x3ec($16)
	.word	0x46000834
	nop	
	.word	0x4500000d
	lbu	$2,0x80($29)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E480
