	.text
	.p2align 3
	.globl	girlBrain_sub_16FD84
	.ent	girlBrain_sub_16FD84
girlBrain_sub_16FD84:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sw	$2,0xc0($29)
	sw	$3,0xc8($29)
	addiu	$2,$29,0x30
	sw	$4,0xb4($29)
	addiu	$3,$29,0x50
	addiu	$4,$29,0x60
	sw	$2,0xb8($29)
	sw	$3,0xbc($29)
	addiu	$23,$20,0x110
	sw	$4,0xc4($29)
	addiu	$30,$29,0x40
	lui	$3,0x28
	lw	$2,0x30($20)
	addiu	$7,$3,0x2ac0
	addiu	$3,$0,4
	xori	$2,$2,0x6b
	lw	$4,0x1f60($7)
	beqz	$4,loc_0016fe30
	movz	$18,$3,$2
	lui	$2,0x27
	addiu	$5,$0,0xa
	lw	$4,0x4ec0($2)
	addiu	$6,$2,0x4ec0
	lw	$3,4($6)
	addiu	$2,$0,0x3c
	.word	0x00852018
	beql	$3,$0,loc_0016fdf4
	break	0,7
loc_0016fdf4:
	subu	$2,$2,$4
	div	$0,$2,$3
	mflo	$2
	srl	$4,$2,0x1f
	addu	$2,$2,$4
	lw	$4,0xb0($29)
	sra	$2,$2,1
	div	$0,$4,$2
	lw	$2,0xc0($29)
	mfhi	$3
	bnez	$3,loc_0016fe3c
	sw	$2,0xb0($29)
	move	$3,$2
	addiu	$3,$3,1
	sw	$3,0xc0($29)
loc_0016fe30:
	addiu	$2,$0,1
	b	loc_0016fe48
	sb	$2,0x58e0($7)
loc_0016fe3c:
	lw	$4,0xc0($29)
	addiu	$4,$4,1
	sw	$4,0xc0($29)
loc_0016fe48:
	lw	$5,0($29)
	jal	0x00104508
	lw	$4,0xc8($29)
	lw	$5,0($29)
	jal	0x0010a498
	lw	$4,0xb4($29)
	sltiu	$2,$18,5
	beqz	$2,loc_001702ac
	lui	$3,0x56
	sll	$21,$18,2
	addiu	$2,$3,-0x6950
	addu	$2,$21,$2
	lw	$3,0($2)
	jr	$3
	nop	
	lw	$5,0($29)
	addiu	$18,$0,1
	lw	$4,0xb8($29)
	jal	0x0010a498
	addiu	$21,$0,4
	lwc1	$f1,0x30($29)
	lui	$4,0x28
	lwc1	$f2,0x34($29)
	addiu	$2,$4,0x2ac0
	lwc1	$f0,0x38($29)
	swc1	$f1,0x5800($2)
	swc1	$f0,0x5808($2)
	swc1	$f2,0x5804($2)
	b	loc_001702b0
	sw	$0,0x33c($20)
	lw	$5,0($29)
	lw	$4,0xb8($29)
	jal	0x0010a498
	addiu	$18,$0,2
	lui	$2,0x29
	lw	$4,0xb8($29)
	addiu	$5,$2,-0x7d60
	jal	0x0016f410
	addiu	$6,$5,0x20
	beql	$2,$0,loc_0016feec
	addiu	$18,$0,3
loc_0016feec:
	b	loc_001702ac
	sw	$0,0x33c($20)
	lui	$3,0x29
	lw	$4,0($29)
	addiu	$16,$3,-0x7d60
	move	$5,$23
	jal	0x0016f9a8
	move	$6,$16
	move	$3,$2
	addiu	$2,$0,1
	beq	$3,$2,loc_0016ff38
	slti	$2,$3,2
	bnez	$2,loc_0016ff7c
	move	$4,$30
	addiu	$2,$0,2
	beq	$3,$2,loc_0016ff58
	lwc1	$f1,0x20($29)
	b	loc_0016ff80
	move	$5,$0
loc_0016ff38:
	addiu	$2,$16,-0x57e0
	lwc1	$f1,0x57e0($2)
	addiu	$18,$0,1
	lwc1	$f2,0x57e4($2)
	addiu	$21,$0,4
	lwc1	$f0,0x57e8($2)
	b	loc_0016ff70
	swc1	$f1,0x5800($2)
loc_0016ff58:
	addiu	$2,$16,-0x57e0
	lwc1	$f2,0x24($29)
	lwc1	$f0,0x28($29)
	addiu	$18,$0,1
	swc1	$f1,0x5800($2)
	addiu	$21,$0,4
loc_0016ff70:
	swc1	$f0,0x5808($2)
	swc1	$f2,0x5804($2)
	move	$4,$30
loc_0016ff7c:
	move	$5,$0
loc_0016ff80:
	addiu	$6,$0,0x10
	jal	0x002641d8
	move	$16,$0
	lui	$1,0x3f80
	mtc1	$1,$f0
	lui	$4,0x28
	addiu	$3,$4,0x2ac0
	swc1	$f0,0xc($30)
	lw	$2,0x1f60($3)
	blezl	$2,loc_00170034
	lui	$3,0x28
	move	$19,$3
	lwc1	$f20,-0x7b38($28)
	addiu	$17,$19,0x1f80
loc_0016ffb8:
	addiu	$4,$29,0x10
	jal	0x00194398
	move	$5,$17
	.word	0x46140034
	nop	
	bc1f	loc_0017001c
	addiu	$4,$29,0x40
	move	$5,$17
	jal	0x00243ae8
	addiu	$6,$29,0x10
	move	$4,$23
	jal	0x00194678
	addiu	$5,$29,0x40
	bgez	$2,loc_00170008
	move	$4,$23
	jal	0x00194678
	addiu	$5,$29,0x40
	negu	$2,$2
	b	loc_00170014
	slti	$2,$2,0x2d
loc_00170008:
	jal	0x00194678
	addiu	$5,$29,0x40
	slti	$2,$2,0x2d
loc_00170014:
	bnez	$2,loc_00170034
	lui	$3,0x28
loc_0017001c:
	lw	$2,0x1f60($19)
	addiu	$16,$16,1
	slt	$2,$16,$2
	bnez	$2,loc_0016ffb8
	addiu	$17,$17,0x30
	lui	$3,0x28
loc_00170034:
	addiu	$2,$3,0x2ac0
	lw	$3,0x58e8($2)
	beqz	$3,loc_00170054
	addiu	$2,$0,1
	beq	$3,$2,loc_00170080
	nop	
	b	loc_00170090
	lwc1	$f0,0x33c($20)
loc_00170054:
	lw	$4,0($29)
	lui	$1,0x3f80
	mtc1	$1,$f21
	lui	$1,0x3f00
	mtc1	$1,$f20
	jal	0x00201d50
	lw	$16,0x164($4)
	beql	$2,$0,loc_0017008c
	swc1	$f21,0x33c($16)
	b	loc_0017008c
	swc1	$f20,0x33c($16)
loc_00170080:
	lui	$1,0x3f00
	mtc1	$1,$f0
	swc1	$f0,0x33c($20)
loc_0017008c:
	lwc1	$f0,0x33c($20)
loc_00170090:
	mtc1	$0,$f20
	.word	0x46140032
	nop	
	bc1t	loc_001701f8
	lui	$3,0x29
	lui	$2,0x29
	lui	$1,0x4396
	mtc1	$1,$f12
	addiu	$19,$2,-0x7d20
	move	$4,$30
	move	$5,$23
	jal	0x00243b18
	addiu	$16,$19,-0x25e0
	move	$17,$0
	move	$4,$30
	move	$5,$19
	jal	0x00243ad0
	move	$6,$30
	lw	$22,-0x25f0($19)
	addiu	$4,$29,0x50
	move	$5,$19
	jal	0x00243ae8
	move	$6,$30
	swc1	$f20,0x54($29)
	addiu	$4,$29,0x60
	jal	0x001940b0
	addiu	$5,$29,0x50
	lwc1	$f12,0x50($29)
	lwc1	$f1,0x54($29)
	mul.s	$f12,$f12,$f12
	lwc1	$f0,0x58($29)
	mul.s	$f1,$f1,$f1
	mul.s	$f0,$f0,$f0
	add.s	$f12,$f12,$f1
	jal	0x00105fe0
	add.s	$f12,$f12,$f0
	blez	$22,loc_001701d4
	mov.s	$f21,$f0
	lwc1	$f22,-0x7b34($28)
	nop	
loc_00170130:
	lwc1	$f1,4($19)
	lwc1	$f0,0x14($16)
	sub.s	$f0,$f0,$f1
	cvt.w.s	$f1,$f0
	mfc1	$2,$f1
	nop	
	bltzl	$2,loc_00170150
	negu	$2,$2
loc_00170150:
	slti	$2,$2,0xc9
	beqz	$2,loc_001701c4
	addiu	$4,$29,0xa0
	addiu	$5,$16,0x10
	jal	0x00243ae8
	addiu	$6,$29,0x40
	addiu	$4,$29,0xa0
	sw	$0,0xa4($29)
	sw	$0,0xac($29)
	addiu	$5,$29,0x60
	jal	0x002438b8
	move	$6,$4
	lwc1	$f0,0xa8($29)
	.word	0x4600a034
	nop	
	bc1fl	loc_001701c8
	addiu	$17,$17,1
	.word	0x46150034
	nop	
	bc1f	loc_001701c4
	lwc1	$f0,0xa0($29)
	lwc1	$f1,0xa4($29)
	mul.s	$f0,$f0,$f0
	mul.s	$f1,$f1,$f1
	add.s	$f0,$f0,$f1
	.word	0x46160034
	nop	
	bc1t	loc_001701d8
	addiu	$2,$0,1
loc_001701c4:
	addiu	$17,$17,1
loc_001701c8:
	slt	$2,$17,$22
	bnez	$2,loc_00170130
	addiu	$16,$16,0x30
loc_001701d4:
	move	$2,$0
loc_001701d8:
	andi	$2,$2,0xff
	beqz	$2,loc_001701f4
	lui	$2,0x28
	sw	$0,0x33c($20)
	addiu	$3,$2,0x2ac0
	addiu	$2,$0,1
	sb	$2,0x58e0($3)
loc_001701f4:
	lui	$3,0x29
loc_001701f8:
	lui	$1,0xbf80
	mtc1	$1,$f12
	addiu	$16,$3,-0x7d60
	lw	$4,0xbc($29)
	jal	0x00243b18
	move	$5,$16
	addiu	$16,$16,-0x57e0
	lwc1	$f13,0x58e4($16)
	cvt.s.w	$f13,$f13
	lui	$1,0x4348
	mtc1	$1,$f12
	lw	$4,0xbc($29)
	addiu	$5,$0,0xff
	move	$6,$0
	jal	0x001873a0
	move	$7,$0
	lw	$2,0x58e4($16)
	addiu	$2,$2,5
	b	loc_001702b0
	sw	$2,0x58e4($16)
	sw	$0,0x33c($20)
	addiu	$5,$0,0xd5
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
	lui	$1,0x4110
	mtc1	$1,$f0
	lui	$4,0x29
	addiu	$3,$4,-0x5770
	ori	$2,$0,0xc000
	dsll32	$2,$2,8
	swc1	$f0,0x18($3)
	ld	$3,0x18($20)
	and	$2,$3,$2
	beqz	$2,loc_001702b0
	nop	
	ori	$2,$0,0x8000
	dsll32	$2,$2,0xa
	or	$2,$3,$2
	b	loc_001702b0
	sd	$2,0x18($20)
	lw	$2,0x30($20)
	sw	$0,0x33c($20)
	xori	$2,$2,0x6b
	movn	$18,$0,$2
loc_001702ac:
	sll	$21,$18,2
loc_001702b0:
	jal	0x00104f20
	nop	
	mtc1	$0,$f20
	lw	$5,-0x6f80($28)
	lui	$2,0x6b
	lw	$4,0xc4($29)
	addiu	$17,$2,-0x4420
	jal	0x002439b0
	addiu	$5,$5,0x80
	swc1	$f20,0x8c($29)
	swc1	$f20,0x7c($29)
	jal	0x00105278
	swc1	$f20,0x6c($29)
	jal	0x00243bd8
	move	$4,$2
	lwc1	$f12,0x10($29)
	lwc1	$f13,0x14($29)
	jal	0x00105308
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16FD84
