	.text
	.p2align 3
	.globl	girlBrain_sub_16FB4C
	.ent	girlBrain_sub_16FB4C
girlBrain_sub_16FB4C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x00168448
	move	$4,$17
	lw	$2,0x168($29)
	beqz	$2,loc_0016fb64
	addiu	$3,$0,1
	move	$3,$0
loc_0016fb64:
	.word	0x1060002e
	lwc1	$f2,0($29)
	move	$4,$18
	lwc1	$f0,8($29)
	addiu	$5,$29,0x30
	lwc1	$f1,0x20($29)
	lwc1	$f3,0x28($29)
	sub.s	$f2,$f2,$f1
	sw	$0,0x34($29)
	sub.s	$f0,$f0,$f3
	swc1	$f2,0x30($29)
	jal	0x00243978
	swc1	$f0,0x38($29)
	lwc1	$f2,4($29)
	lwc1	$f0,0x24($29)
	mtc1	$0,$f1
	sub.s	$f2,$f2,$f0
	.word	0x46011034
	nop	
	bc1f	loc_0016fbdc
	move	$4,$29
	neg.s	$f1,$f2
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0016fbf4
	move	$3,$0
	.word	0x10000010
	andi	$3,$3,0xff
loc_0016fbdc:
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46001034
	nop	
	.word	0x45000009
	move	$3,$0
loc_0016fbf4:
	jal	0x00194398
	addiu	$5,$29,0x20
	lwc1	$f1,-0x7b3c($28)
	.word	0x46010034
	nop	
	.word	0x45010002
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16FB4C
