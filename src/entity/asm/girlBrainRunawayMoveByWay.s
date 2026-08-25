	.text
	.p2align 3
	.globl	girlBrainRunawayMoveByWay
	.ent	girlBrainRunawayMoveByWay
girlBrainRunawayMoveByWay:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x200
	lwc1	$f0,0($6)
	sd	$18,0x1c0($29)
	sd	$31,0x1f0($29)
	move	$18,$5
	sd	$20,0x1e0($29)
	sd	$19,0x1d0($29)
	sd	$16,0x1a0($29)
	sd	$17,0x1b0($29)
	lw	$17,0x164($4)
	lwc1	$f2,8($6)
	lwc1	$f1,4($6)
	lw	$3,0x394($17)
	swc1	$f0,0($29)
	swc1	$f1,4($29)
	beqz	$3,loc_0016fa00
	swc1	$f2,8($29)
	addiu	$2,$0,1
	beq	$3,$2,loc_0016fb00
	move	$2,$0
	b	loc_0016fc28
	ld	$31,0x1f0($29)
loc_0016fa00:
	addiu	$16,$29,0x10
	move	$5,$4
	move	$4,$16
	jal	0x0010a498
	addiu	$19,$17,0x350
	move	$20,$0
	lwc1	$f0,0x14($29)
	move	$5,$16
	mtc1	$0,$f1
	move	$4,$19
	sub.s	$f0,$f0,$f1
	jal	0x00177ca0
	swc1	$f0,0x14($29)
	move	$16,$2
	bnez	$16,loc_0016fa50
	move	$4,$18
	jal	0x00243b60
	addiu	$5,$17,0x3a0
	b	loc_0016fa84
	lw	$2,0x378($17)
loc_0016fa50:
	jal	0x00243b60
	addiu	$5,$17,0x3a0
	lui	$2,0x28
	addiu	$3,$2,0x2ac0
	lw	$2,0x5850($3)
	beql	$2,$16,loc_0016fa84
	lw	$2,0x378($17)
	beql	$2,$0,loc_0016fa80
	sw	$16,0x5850($3)
	lw	$2,0x3b4($17)
	slti	$20,$2,1
	sw	$16,0x5850($3)
loc_0016fa80:
	lw	$2,0x378($17)
loc_0016fa84:
	bnez	$2,loc_0016faf0
	addiu	$16,$29,0x20
	lui	$1,0x4120
	mtc1	$1,$f0
	move	$4,$16
	addiu	$5,$29,0x10
	jal	0x00243b60
	swc1	$f0,0x70($16)
	addiu	$4,$29,0x30
	jal	0x00243b60
	move	$5,$29
	jal	0x001683c8
	move	$4,$16
	lw	$2,0xa8($29)
	bnez	$2,loc_0016fadc
	move	$3,$0
	jal	0x00168448
	move	$4,$16
	lw	$2,0xa8($29)
	beqz	$2,loc_0016fadc
	addiu	$3,$0,1
	move	$3,$0
loc_0016fadc:
	beqz	$3,loc_0016faf0
	addiu	$2,$0,1
	move	$4,$19
	jal	0x00178930
	sw	$2,0x394($17)
loc_0016faf0:
	beqz	$20,loc_0016fc20
	addiu	$2,$0,2
	b	loc_0016fc28
	ld	$31,0x1f0($29)
loc_0016fb00:
	addiu	$16,$29,0x20
	move	$5,$4
	jal	0x00104508
	move	$4,$16
	lui	$1,0x4120
	mtc1	$1,$f0
	addiu	$17,$29,0xe0
	move	$5,$16
	move	$4,$17
	jal	0x00243b60
	swc1	$f0,0x70($17)
	addiu	$4,$29,0xf0
	jal	0x00243b60
	move	$5,$29
	jal	0x001683c8
	move	$4,$17
	lw	$2,0x168($29)
	bnez	$2,loc_0016fb64
	move	$3,$0
	jal	0x00168448
	move	$4,$17
	lw	$2,0x168($29)
	beqz	$2,loc_0016fb64
	addiu	$3,$0,1
	move	$3,$0
loc_0016fb64:
	beqz	$3,loc_0016fc20
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
	b	loc_0016fc18
	andi	$3,$3,0xff
loc_0016fbdc:
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46001034
	nop	
	bc1f	loc_0016fc14
	move	$3,$0
loc_0016fbf4:
	jal	0x00194398
	addiu	$5,$29,0x20
	lwc1	$f1,-0x7b3c($28)
	.word	0x46010034
	nop	
	bc1t	loc_0016fc14
	addiu	$3,$0,1
	move	$3,$0
loc_0016fc14:
	andi	$3,$3,0xff
loc_0016fc18:
	bnez	$3,loc_0016fc24
	addiu	$2,$0,1
loc_0016fc20:
	move	$2,$0
loc_0016fc24:
	ld	$31,0x1f0($29)
loc_0016fc28:
	ld	$20,0x1e0($29)
	ld	$19,0x1d0($29)
	ld	$18,0x1c0($29)
	ld	$17,0x1b0($29)
	ld	$16,0x1a0($29)
	jr	$31
	addiu	$29,$29,0x200
	nop	
	lui	$2,0x27
	addiu	$7,$0,0xa
	lw	$5,0x4ec0($2)
	.set	macro
	.set	reorder
	.end	girlBrainRunawayMoveByWay
