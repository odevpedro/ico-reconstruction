	.text
	.p2align 3
	.globl	girlBrain_sub_16DADC
	.ent	girlBrain_sub_16DADC
girlBrain_sub_16DADC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$9,8($2)
	sd	$5,0x10($2)
	sd	$6,0x18($2)
	addiu	$3,$3,0x20
	addiu	$2,$2,0x20
	nop	
	.word	0x1464fff4
	nop	
	move	$5,$7
	sw	$7,0x30($29)
	jal	0x0010a498
	move	$4,$19
	nop	
	lw	$4,0($29)
	addiu	$20,$0,1
	sw	$0,0xf4($29)
	jal	0x0015eff8
	move	$23,$0
	lwc1	$f0,0($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0xa0($29)
	lwc1	$f0,4($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0xa4($29)
	lwc1	$f0,8($2)
	jal	0x0014a0d8
	swc1	$f0,0xa8($29)
	bnez	$2,loc_0016db68
	lw	$4,0x94($29)
	lui	$3,0x20
	ld	$2,0x20($4)
	or	$2,$2,$3
	sd	$2,0x20($4)
loc_0016db68:
	bnez	$22,loc_0016db90
	lw	$3,0x30($29)
	lw	$2,-0x6e0c($28)
	bne	$3,$2,loc_0016db90
	nop	
	lw	$2,0($29)
	lw	$3,0x164($2)
	lw	$4,0x678($3)
	lw	$2,0x380($4)
	movn	$20,$0,$2
loc_0016db90:
	lw	$2,0($29)
	lw	$3,0x15c($2)
	lw	$4,0($3)
	beqz	$4,loc_0016dc54
	lui	$7,0x28
	lw	$4,0($29)
	addiu	$5,$0,0x11
	lw	$2,0x15c($4)
	lw	$3,0($2)
	lw	$4,0xc($3)
	bne	$4,$5,loc_0016dc58
	lw	$3,0x94($29)
	lw	$2,-0x6e0c($28)
	beqz	$2,loc_0016dc58
	nop	
	.word	0x56c00023
	addiu	$2,$7,0x2ac0
	lw	$4,0x30($29)
	.word	0x14820020
	addiu	$2,$7,0x2ac0
	jal	0x0015eff8
	nop	
	lwc1	$f1,4($2)
	lwc1	$f0,0xa4($29)
	.word	0x46010034
	nop	
	bc1f	loc_0016dc54
	lui	$7,0x28
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	lw	$4,-0x6e0c($28)
	jal	0x0015eff8
	move	$16,$2
	move	$4,$16
	jal	0x00194398
	move	$5,$2
	mov.s	$f21,$f0
	jal	0x0014b358
	addiu	$4,$0,0x1c
	mov.s	$f20,$f0
	jal	0x0014b358
	addiu	$4,$0,0x1c
	mul.s	$f20,$f20,$f0
	.word	0x4614a834
	nop	
	bc1tl	loc_0016dc50
	addiu	$23,$0,1
loc_0016dc50:
	lui	$7,0x28
loc_0016dc54:
	lw	$3,0x94($29)
loc_0016dc58:
	addiu	$2,$7,0x2ac0
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16DADC
