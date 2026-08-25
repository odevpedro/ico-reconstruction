	.text
	.p2align 3
	.globl	girlBrain_sub_16E558
	.ent	girlBrain_sub_16E558
girlBrain_sub_16E558:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	swc1	$f20,0x33c($16)
	lw	$2,0x94($29)
	lwc1	$f1,0x84($29)
	lwc1	$f0,0x3e8($2)
	.word	0x46010034
	nop	
	bc1f	loc_0016e5a0
	lw	$4,0x108($29)
	lwc1	$f0,0x33c($2)
	lui	$1,0x3f00
	mtc1	$1,$f2
	.word	0x46001034
	nop	
	bc1f	loc_0016e598
	mov.s	$f1,$f0
	mov.s	$f1,$f2
loc_0016e598:
	swc1	$f1,0x33c($2)
	lw	$4,0x108($29)
loc_0016e5a0:
	jal	0x00194398
	move	$5,$19
	lwc1	$f1,-0x7b50($28)
	.word	0x46010034
	nop	
	.word	0x45010017
	lw	$5,0x30($29)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E558
