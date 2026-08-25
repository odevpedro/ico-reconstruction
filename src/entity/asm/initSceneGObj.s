	.text
	.p2align 3
	.globl	initSceneGObj
	.ent	initSceneGObj
initSceneGObj:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x130
	addiu	$3,$0,0x4c
	sd	$22,0xf0($29)
	lui	$2,0x2a
	move	$22,$5
	sd	$23,0x100($29)
	mult	$3,$22,$3
	sd	$21,0xe0($29)
	sd	$20,0xd0($29)
	addiu	$2,$2,0x4c48
	sd	$18,0xb0($29)
	addiu	$6,$0,0x64
	sd	$17,0xa0($29)
	lui	$18,0x2a
	sd	$31,0x120($29)
	addu	$20,$3,$2
	sd	$30,0x110($29)
	move	$17,$4
	sd	$19,0xc0($29)
	addiu	$7,$18,0x31b8
	sd	$16,0x90($29)
	lbu	$4,0x46($20)
	.word	0x00863018
	jal	0x001ae5f0
	addu	$23,$6,$7
	lw	$3,0x2c($20)
	move	$21,$2
	beqz	$21,loc_001b7810
	sw	$3,0x80($29)
	lhu	$16,4($21)
	bne	$16,$17,loc_001b7b5c
	ld	$31,0x120($29)
	addiu	$4,$0,0x194
	lui	$2,0x5f
	.word	0x02042018
	addiu	$2,$2,0x2fb8
	ld	$3,0($21)
	addiu	$5,$0,1
	addiu	$6,$0,1
	or	$3,$3,$5
	addu	$4,$4,$2
	lw	$2,0x190($4)
	srl	$2,$2,1
	andi	$2,$2,1
	bne	$2,$6,loc_001b7810
	sd	$3,0($21)
	jal	0x001ae808
	nop	
	beq	$2,$16,loc_001b7810
	addiu	$2,$0,0xf
	lbu	$4,0x46($20)
	beq	$4,$2,loc_001b7804
	slti	$2,$4,0x10
	beqz	$2,loc_001b77e4
	addiu	$2,$0,4
	beq	$4,$2,loc_001b77f8
	nop	
	b	loc_001b7814
	lui	$4,0x61
loc_001b77e4:
	addiu	$2,$0,0x21
	beq	$4,$2,loc_001b7808
	move	$5,$22
	b	loc_001b7814
	lui	$4,0x61
loc_001b77f8:
	jal	0x00193e48
	move	$4,$22
	lbu	$4,0x46($20)
loc_001b7804:
	move	$5,$22
loc_001b7808:
	jal	0x001ae6f8
	move	$21,$0
loc_001b7810:
	lui	$4,0x61
loc_001b7814:
	lw	$6,0x80($29)
	move	$5,$22
	jal	0x001a6e28
	addiu	$4,$4,0x7260
	lw	$2,0x44($23)
	beqz	$2,loc_001b7b50
	move	$30,$0
	lwc1	$f4,0xc($20)
	lwc1	$f3,0x14($20)
	lwc1	$f9,-0x77e0($28)
	lwc1	$f1,0x1c($20)
	mul.s	$f4,$f4,$f9
	lwc1	$f2,0x20($20)
	mul.s	$f3,$f3,$f9
	lui	$1,0x4334
	mtc1	$1,$f8
	lui	$1,0x3f80
	mtc1	$1,$f5
	neg.s	$f1,$f1
	lwc1	$f0,0x18($20)
	neg.s	$f2,$f2
	lwc1	$f7,0($20)
	nop	
	nop	
	div.s	$f4,$f4,$f8
	swc1	$f5,0x4c($29)
	nop	
	nop	
	div.s	$f3,$f3,$f8
	neg.s	$f0,$f0
	lwc1	$f6,4($20)
	swc1	$f2,0x48($29)
	swc1	$f1,0x44($29)
	swc1	$f0,0x40($29)
	lwc1	$f0,8($20)
	swc1	$f4,0x50($29)
	swc1	$f3,0x58($29)
	sw	$0,0x54($29)
	sw	$0,0x5c($29)
	ld	$3,0x40($29)
	lw	$2,0x38($20)
	ld	$4,0x48($29)
	ld	$5,0x50($29)
	ld	$6,0x58($29)
	sw	$2,0x70($29)
	swc1	$f7,0x60($29)
	swc1	$f6,0x64($29)
	swc1	$f0,0x68($29)
	swc1	$f5,0x6c($29)
	sd	$3,0($29)
	sd	$4,8($29)
	sd	$5,0x10($29)
	sd	$6,0x18($29)
	lwc1	$f1,0x10($20)
	ld	$5,0x60($29)
	ld	$3,0x68($29)
	.word	0x46014034
	ld	$4,0x70($29)
	mov.s	$f2,$f1
	ld	$2,0x78($29)
	sd	$5,0x20($29)
	sd	$3,0x28($29)
	sd	$4,0x30($29)
	bc1f	loc_001b7924
	sd	$2,0x38($29)
	lui	$1,0x43b4
	mtc1	$1,$f0
	sub.s	$f1,$f1,$f0
loc_001b7924:
	lui	$1,0xc334
	mtc1	$1,$f0
	.word	0x46001034
	nop	
	bc1fl	loc_001b794c
	mul.s	$f0,$f1,$f9
	lui	$1,0x43b4
	mtc1	$1,$f0
	add.s	$f1,$f1,$f0
	mul.s	$f0,$f1,$f9
loc_001b794c:
	nop	
	nop	
	div.s	$f0,$f0,$f8
	beqz	$21,loc_001b79b0
	swc1	$f0,0x14($29)
	lw	$2,0x38($23)
	beqz	$2,loc_001b797c
	move	$4,$29
	jalr	$2
	move	$5,$21
	b	loc_001b79b4
	lw	$3,-0x6f60($28)
loc_001b797c:
	lwc1	$f1,0x10($21)
	lw	$30,0x30($21)
	swc1	$f1,0($29)
	lwc1	$f0,0x14($21)
	swc1	$f0,4($29)
	lwc1	$f1,0x18($21)
	swc1	$f1,8($29)
	lwc1	$f0,0x20($21)
	swc1	$f0,0x10($29)
	lwc1	$f1,0x24($21)
	swc1	$f1,0x14($29)
	lwc1	$f0,0x28($21)
	swc1	$f0,0x18($29)
loc_001b79b0:
	lw	$3,-0x6f60($28)
loc_001b79b4:
	lw	$2,-0x5764($28)
	bne	$3,$2,loc_001b7a14
	lbu	$5,0x46($20)
	addiu	$2,$0,1
	bne	$5,$2,loc_001b7a18
	addiu	$16,$0,0x64
	lui	$2,0x70
	lwc1	$f1,-0x77dc($28)
	lwc1	$f0,0x6f64($2)
	lui	$3,0x70
	lui	$1,0x4334
	mtc1	$1,$f4
	addiu	$2,$3,0x6f50
	mul.s	$f0,$f0,$f1
	lwc1	$f3,8($2)
	lwc1	$f1,4($2)
	lwc1	$f2,0x6f50($3)
	nop	
	nop	
	div.s	$f0,$f0,$f4
	swc1	$f1,4($29)
	swc1	$f2,0($29)
	swc1	$f3,8($29)
	swc1	$f0,0x14($29)
loc_001b7a14:
	addiu	$16,$0,0x64
loc_001b7a18:
	lw	$7,0x48($20)
	.word	0x00b08018
	addiu	$2,$18,0x31b8
	lbu	$17,0x47($20)
	srl	$7,$7,0xe
	lw	$18,0x30($20)
	andi	$7,$7,7
	andi	$17,$17,0x1f
	move	$6,$22
	addu	$16,$16,$2
	move	$8,$0
	jal	0x00240d40
	move	$4,$16
	lw	$4,0x80($29)
	move	$19,$2
	jal	0x0019f310
	move	$5,$29
	sw	$2,0x15c($19)
	move	$5,$17
	move	$4,$19
	sw	$18,0x814($2)
	jal	0x00115108
	addiu	$6,$0,1
	lw	$2,0x58($16)
	beqz	$2,loc_001b7a90
	move	$4,$19
	jalr	$2
	move	$5,$29
	lw	$3,0x15c($19)
	sw	$2,0x800($3)
loc_001b7a90:
	lhu	$2,0x40($20)
	beqz	$2,loc_001b7aa0
	addiu	$8,$0,0x1800
	dsll	$8,$2,0xa
loc_001b7aa0:
	lw	$5,0x24($20)
	beqz	$5,loc_001b7ac0
	move	$4,$19
	move	$6,$0
	jal	0x0013f7a8
	addiu	$7,$0,0x13
	b	loc_001b7ad8
	lbu	$3,0x46($20)
loc_001b7ac0:
	lw	$5,0x40($23)
	beqz	$5,loc_001b7ad4
	move	$6,$0
	jal	0x0013f7a8
	addiu	$7,$0,0x13
loc_001b7ad4:
	lbu	$3,0x46($20)
loc_001b7ad8:
	addiu	$2,$0,1
	beql	$3,$2,loc_001b7ae4
	sw	$19,-0x6e0c($28)
loc_001b7ae4:
	addiu	$2,$0,2
	beql	$3,$2,loc_001b7af0
	sw	$19,-0x6e08($28)
loc_001b7af0:
	beqz	$21,loc_001b7b10
	addiu	$2,$0,4
	lw	$6,0x34($23)
	beqz	$6,loc_001b7b10
	move	$5,$21
	jalr	$6
	move	$4,$19
	addiu	$2,$0,4
loc_001b7b10:
	beql	$30,$2,loc_001b7b18
	sw	$19,-0x6068($28)
loc_001b7b18:
	lw	$2,0x28($20)
	bnel	$2,$0,loc_001b7b24
	sw	$19,0($2)
loc_001b7b24:
	lui	$4,0x29
	move	$6,$22
	addiu	$4,$4,-0x5770
	jal	0x00182000
	move	$5,$19
	lbu	$5,0x46($20)
	jal	0x001918f0
	move	$4,$19
	move	$4,$19
	jal	0x00203cb8
	move	$5,$30
loc_001b7b50:
	jal	0x00166028
	nop	
	ld	$31,0x120($29)
loc_001b7b5c:
	ld	$30,0x110($29)
	ld	$23,0x100($29)
	ld	$22,0xf0($29)
	ld	$21,0xe0($29)
	ld	$20,0xd0($29)
	ld	$19,0xc0($29)
	ld	$18,0xb0($29)
	ld	$17,0xa0($29)
	ld	$16,0x90($29)
	jr	$31
	addiu	$29,$29,0x130
	move	$7,$4
	addiu	$3,$0,0x4c
	mult	$3,$7,$3
	addiu	$29,$29,-0x50
	lui	$2,0x2a
	sd	$18,0x20($29)
	sd	$31,0x40($29)
	addiu	$2,$2,0x4c48
	sd	$19,0x30($29)
	addiu	$5,$0,0x64
	sd	$17,0x10($29)
	addu	$3,$3,$2
	sd	$16,0($29)
	lui	$4,0x2a
	addiu	$4,$4,0x31b8
	lbu	$6,0x46($3)
	.word	0x00c52818
	addu	$18,$5,$4
	lw	$2,0x44($18)
	beqz	$2,loc_001b7cbc
	lhu	$16,0x44($3)
	beqz	$16,loc_001b7cbc
	addiu	$2,$0,4
	beq	$6,$2,loc_001b7cc0
	ld	$31,0x40($29)
	jal	0x0013eae8
	move	$4,$7
	move	$19,$2
	jal	0x0013eae8
	move	$4,$16
	move	$17,$2
	beql	$17,$0,loc_001b7c70
	lui	$4,0x61
	bne	$17,$19,loc_001b7c50
	lui	$4,0x61
	lui	$4,0x61
	move	$5,$18
	jal	0x001a6e28
	addiu	$4,$4,0x7298
	lui	$16,0x61
	addiu	$5,$0,0x1e7
	addiu	$16,$16,0x72e0
	jal	0x001ad768
	move	$4,$16
	lui	$6,0x63
	move	$4,$16
	addiu	$6,$6,0x3190
	jal	0x00263ff0
	addiu	$5,$0,0x1e7
	lui	$4,0x61
loc_001b7c50:
	move	$5,$18
	jal	0x001a6e28
	addiu	$4,$4,0x72f8
	lw	$2,0x15c($19)
	sw	$17,0($2)
	lw	$3,0x15c($19)
	b	loc_001b7cbc
	sw	$0,4($3)
loc_001b7c70:
	move	$5,$18
	jal	0x001a6e28
	addiu	$4,$4,0x7308
	lui	$16,0x61
	addiu	$5,$0,0x1f0
	addiu	$16,$16,0x72e0
	jal	0x001ad768
	move	$4,$16
	move	$4,$16
	lui	$6,0x63
	ld	$31,0x40($29)
	addiu	$6,$6,0x3190
	ld	$19,0x30($29)
	addiu	$5,$0,0x1f0
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x263ff0
	addiu	$29,$29,0x50
loc_001b7cbc:
	ld	$31,0x40($29)
loc_001b7cc0:
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x50
	j	0x206328
	nop	
	addiu	$29,$29,-0x60
	sd	$18,0x20($29)
	move	$18,$4
	sd	$20,0x40($29)
	sd	$16,0($29)
	sd	$31,0x50($29)
	addiu	$16,$0,2
	sd	$19,0x30($29)
	jal	0x00240b60
	sd	$17,0x10($29)
	lui	$4,0x61
	sw	$0,-0x6e08($28)
	addiu	$4,$4,0x7348
	sw	$0,-0x6e0c($28)
	sw	$0,-0x6e04($28)
	move	$5,$18
	sw	$0,-0x6e00($28)
	jal	0x001a6e28
	sw	$0,-0x6dfc($28)
	lui	$1,0x3f80
	mtc1	$1,$f12
	lui	$1,0x447a
	mtc1	$1,$f13
	jal	0x00114e08
	nop	
	jal	0x00181f58
	nop	
	jal	0x0014a2b0
	nop	
	jal	0x001ae3b0
	nop	
	jal	0x00192088
	sw	$0,-0x6068($28)
	jal	0x001c36e0
	nop	
	jal	0x001f22a0
	nop	
	move	$4,$18
	move	$5,$0
	jal	0x00240c50
	addiu	$6,$0,1
	move	$20,$2
	move	$5,$16
	nop	
loc_001b7d90:
	jal	0x001b76f8
	move	$4,$18
	addiu	$16,$16,1
	slti	$2,$16,6
	bnez	$2,loc_001b7d90
	move	$5,$16
	addiu	$16,$0,2
	nop	
loc_001b7db0:
	jal	0x001b7b88
	move	$4,$16
	addiu	$16,$16,1
	slti	$2,$16,6
	bnez	$2,loc_001b7db0
	addiu	$3,$0,0x194
	lui	$2,0x5f
	mult	$3,$18,$3
	addiu	$2,$2,0x2fb8
	addu	$4,$2,$3
	move	$2,$4
	lw	$19,0x128($4)
	lw	$17,0x12c($2)
	move	$16,$19
	slt	$2,$16,$17
	beqz	$2,loc_001b7e18
	move	$5,$16
	nop	
loc_001b7df8:
	jal	0x001b76f8
	move	$4,$18
	addiu	$16,$16,1
	slt	$2,$16,$17
	bnez	$2,loc_001b7df8
	move	$5,$16
	move	$16,$19
	slt	$2,$16,$17
loc_001b7e18:
	beqz	$2,loc_001b7e38
	lui	$2,0x4b
loc_001b7e20:
	jal	0x001b7b88
	move	$4,$16
	addiu	$16,$16,1
	slt	$2,$16,$17
	bnez	$2,loc_001b7e20
	lui	$2,0x4b
loc_001b7e38:
	addiu	$17,$0,0xb5
	addiu	$16,$2,0x3d10
loc_001b7e40:
	lhu	$2,2($16)
	beql	$2,$0,loc_001b7e78
	addiu	$17,$17,-1
	ld	$2,0($16)
	andi	$2,$2,1
	bnel	$2,$0,loc_001b7e78
	addiu	$17,$17,-1
	lhu	$2,4($16)
	bnel	$2,$18,loc_001b7e78
	addiu	$17,$17,-1
	lhu	$5,2($16)
	jal	0x001b76f8
	move	$4,$18
	addiu	$17,$17,-1
loc_001b7e78:
	bgez	$17,loc_001b7e40
	addiu	$16,$16,0x40
	lw	$4,-0x6e08($28)
	beql	$4,$0,loc_001b7e98
	lw	$4,-0x6e0c($28)
	jal	0x0013e7f8
	move	$5,$20
	lw	$4,-0x6e0c($28)
loc_001b7e98:
	beqz	$4,loc_001b7ea8
	nop	
	jal	0x0013e7f8
	move	$5,$20
loc_001b7ea8:
	jal	0x0013eb50
	addiu	$4,$0,4
	move	$4,$2
	beqz	$4,loc_001b7ef0
	lui	$16,0x2a
	lw	$3,8($4)
loc_001b7ec0:
	addiu	$2,$0,0x4c
	addiu	$5,$16,0x4c48
	lui	$6,0x20
	mult	$3,$3,$2
	addu	$3,$3,$5
	lw	$2,0x48($3)
	or	$2,$2,$6
	jal	0x0013ebe0
	sw	$2,0x48($3)
	move	$4,$2
	bnel	$4,$0,loc_001b7ec0
	lw	$3,8($4)
loc_001b7ef0:
	jal	0x0018c0a8
	nop	
	jal	0x001b7cd8
	move	$4,$18
	ld	$31,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x168058
	addiu	$29,$29,0x60
	.set	macro
	.set	reorder
	.end	initSceneGObj
