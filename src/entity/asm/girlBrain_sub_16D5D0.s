	.text
	.p2align 3
	.globl	girlBrain_sub_16D5D0
	.ent	girlBrain_sub_16D5D0
girlBrain_sub_16D5D0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x0015eff8
	nop	
	lwc1	$f0,0($2)
	lw	$3,-0x6e0c($28)
	swc1	$f0,0($29)
	lw	$2,0x15c($3)
	jal	0x0015eff8
	lw	$4,0($2)
	lwc1	$f0,4($2)
	lw	$3,-0x6e0c($28)
	swc1	$f0,4($29)
	lw	$2,0x15c($3)
	jal	0x0015eff8
	lw	$4,0($2)
	lwc1	$f0,8($2)
	b	loc_0016d61c
	swc1	$f0,8($29)
	.word	0x10000035
	addiu	$2,$0,1
loc_0016d61c:
	bnez	$17,loc_0016d674
	addiu	$2,$0,2
	beqz	$16,loc_0016d674
	nop	
	addiu	$19,$0,1
	bne	$16,$19,loc_0016d674
	lwc1	$f1,8($29)
	lui	$1,0xc248
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_0016d674
	lwc1	$f0,0x10($29)
	addiu	$19,$0,2
	lui	$1,0x44e1
	mtc1	$1,$f1
	lui	$1,0xc32a
	mtc1	$1,$f2
	swc1	$f0,0($18)
	swc1	$f1,4($18)
	swc1	$f2,8($18)
	addiu	$2,$0,2
loc_0016d674:
	.word	0x5622001d
	move	$2,$19
	.word	0x5211001b
	move	$2,$19
	addiu	$19,$0,1
	.word	0x56130018
	move	$2,$19
	lwc1	$f1,8($29)
	lwc1	$f0,-0x7b6c($28)
	.word	0x46010034
	nop	
	.word	0x45000011
	lwc1	$f1,0($29)
	lwc1	$f0,-0x7b68($28)
	.word	0x46010034
	nop	
	.word	0x4500000d
	move	$2,$19
	lwc1	$f0,-0x7b64($28)
	.word	0x46000834
	nop	
	.word	0x45000008
	lwc1	$f0,0x10($29)
	addiu	$19,$0,2
	lwc1	$f2,-0x7b60($28)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16D5D0
