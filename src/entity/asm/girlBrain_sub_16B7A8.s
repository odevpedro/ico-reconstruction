	.text
	.p2align 3
	.globl	girlBrain_sub_16B7A8
	.ent	girlBrain_sub_16B7A8
girlBrain_sub_16B7A8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$22,0x90($29)
	move	$23,$0
	sd	$21,0x80($29)
	move	$22,$0
	sd	$31,0xc0($29)
	move	$21,$4
	sd	$19,0x60($29)
	move	$4,$20
	sd	$18,0x50($29)
	sd	$16,0x30($29)
	sw	$5,0($29)
	sd	$17,0x40($29)
	jal	0x0015eff8
	lw	$17,0x164($20)
	lwc1	$f0,0($2)
	move	$4,$20
	jal	0x0015eff8
	swc1	$f0,0x10($29)
	lwc1	$f0,4($2)
	move	$4,$20
	jal	0x0015eff8
	swc1	$f0,0x14($29)
	lwc1	$f0,8($2)
	addiu	$3,$0,0x6b
	lw	$2,0x30($17)
	bne	$2,$3,loc_0016b820
	swc1	$f0,0x18($29)
	lw	$4,0($29)
	.word	0x1000010e
	addiu	$3,$0,5
loc_0016b820:
	lui	$2,0x29
	move	$19,$0
	addiu	$16,$2,-0x5770
	jal	0x001823f0
	move	$4,$16
	lh	$3,0x24($16)
	addiu	$5,$0,-1
	beq	$3,$5,loc_0016b850
	addiu	$2,$0,0x1c
	.word	0x00622018
	addu	$2,$4,$16
	lw	$19,0x28($2)
loc_0016b850:
	lui	$18,0x28
	mtc1	$0,$f2
	addiu	$4,$18,0x2ac0
	beq	$3,$5,loc_0016b868
	sw	$19,0x57d0($4)
	lwc1	$f2,0x20($16)
loc_0016b868:
	lui	$1,0x4120
	mtc1	$1,$f0
	lui	$1,0x4040
	mtc1	$1,$f1
	mul.s	$f2,$f2,$f0
	.word	0x46020836
	nop	
	bc1f	loc_0016b898
	lui	$3,0x800
	ld	$2,0x20($17)
	or	$2,$2,$3
	sd	$2,0x20($17)
loc_0016b898:
	lui	$1,0x4000
	mtc1	$1,$f0
	.word	0x46020036
	nop	
	bc1f	loc_0016b8bc
	lui	$3,0x1000
	ld	$2,0x20($17)
	or	$2,$2,$3
	sd	$2,0x20($17)
loc_0016b8bc:
	lw	$6,0x57d0($4)
	lw	$2,0x57d4($4)
	beq	$6,$2,loc_0016b8d4
	nop	
	sw	$6,0x57d4($4)
	addiu	$30,$0,1
loc_0016b8d4:
	beqz	$6,loc_0016b8ec
	move	$4,$20
	lui	$1,0xbf80
	mtc1	$1,$f12
	jal	0x0014b270
	addiu	$5,$0,0xa
loc_0016b8ec:
	sltiu	$2,$21,0xa
	.word	0x104000b8
	lui	$2,0x56
	sll	$3,$21,2
	addiu	$2,$2,-0x6a90
	addu	$3,$3,$2
	lw	$4,0($3)
	jr	$4
	nop	
	addiu	$2,$18,0x2ac0
	lw	$3,0x57d4($2)
	beqz	$3,loc_0016b92c
	lw	$3,0($29)
	addiu	$2,$0,1
	b	loc_0016b934
	sw	$2,0($3)
loc_0016b92c:
	lw	$2,0($29)
	sw	$0,0($2)
loc_0016b934:
	addiu	$2,$18,0x2ac0
	lw	$3,0x3230($2)
	.word	0x106000a5
	lui	$6,0x28
	move	$17,$0
	lw	$3,0x28e8($6)
	addiu	$2,$0,-1
	.word	0x106200a0
	move	$21,$6
	move	$5,$0
	addiu	$2,$6,0x28e8
loc_0016b960:
	addu	$2,$5,$2
	jal	0x0013eb50
	lw	$4,0($2)
	move	$16,$2
	beqz	$16,loc_0016ba40
	addiu	$19,$17,1
	addiu	$17,$29,0x20
	move	$4,$16
loc_0016b980:
	jal	0x0015eff8
	move	$18,$0
	lwc1	$f0,0($2)
	move	$4,$16
	jal	0x0015eff8
	swc1	$f0,0x20($29)
	lwc1	$f0,4($2)
	move	$4,$16
	jal	0x0015eff8
	swc1	$f0,0x24($29)
	lwc1	$f0,8($2)
	move	$4,$20
	mtc1	$0,$f12
	move	$5,$16
	swc1	$f0,0x28($29)
	move	$6,$17
	jal	0x00144e58
	addiu	$7,$0,0xa0
	beqz	$2,loc_0016b9ec
	addiu	$4,$29,0x10
	jal	0x00194398
	move	$5,$17
	lwc1	$f1,-0x7b94($28)
	.word	0x46010034
	nop	
	bc1tl	loc_0016b9ec
	addiu	$18,$0,1
loc_0016b9ec:
	move	$4,$20
	jal	0x0014af70
	move	$5,$16
	bnez	$2,loc_0016ba08
	nop	
	beqz	$18,loc_0016ba24
	nop	
loc_0016ba08:
	jal	0x0016b3c0
	move	$4,$0
	move	$3,$2
	bltz	$3,loc_0016ba40
	lw	$2,0($29)
	b	loc_0016ba40
	sw	$3,0($2)
loc_0016ba24:
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
	bnez	$16,loc_0016b980
	move	$4,$16
	b	loc_0016ba44
	move	$17,$19
loc_0016ba40:
	move	$17,$19
loc_0016ba44:
	move	$6,$21
	sll	$2,$17,2
	addiu	$3,$6,0x28e8
	move	$5,$2
	addiu	$4,$0,-1
	addu	$3,$5,$3
	lw	$2,0($3)
	bne	$2,$4,loc_0016b960
	addiu	$2,$6,0x28e8
	.word	0x1000005a
	nop	
	addiu	$16,$18,0x2ac0
	lbu	$2,0x58e0($16)
	beqz	$2,loc_0016baa0
	lui	$3,0x27
	jal	0x0016b3c0
	addiu	$4,$0,1
	move	$3,$2
	bltz	$3,loc_0016ba98
	lw	$2,0($29)
	sw	$3,0($2)
loc_0016ba98:
	sb	$0,0x58e0($16)
	lui	$3,0x27
loc_0016baa0:
	lw	$2,0x5250($3)
	andi	$2,$2,8
	.word	0x1040004a
	move	$4,$20
	jal	0x0015bcc8
	addiu	$5,$0,0xeb
	.word	0x10000046
	nop	
	addiu	$2,$18,0x2ac0
	lw	$3,0x5904($2)
	.word	0x14600008
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16B7A8
