	.text
	.p2align 3
	.globl	isysGObjRemoveAll
	.ent	isysGObjRemoveAll
isysGObjRemoveAll:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x80
	lw	$6,-0x4c4c($28)
	sd	$31,0x70($29)
	move	$5,$0
	sd	$22,0x60($29)
	sd	$21,0x50($29)
	sd	$20,0x40($29)
	sd	$19,0x30($29)
	sd	$18,0x20($29)
	sd	$17,0x10($29)
	beqz	$6,loc_0013dfc8
	sd	$16,0($29)
	lui	$2,0x6b
	lui	$3,0x55
	addiu	$22,$2,-0x6c30
	addiu	$20,$3,0x7a10
	lui	$21,0x63
	addiu	$2,$0,0x174
loc_0013dee8:
	lw	$4,-0x4c50($28)
	mult	$2,$5,$2
	addu	$17,$2,$4
	lw	$3,0($17)
	beqz	$3,loc_0013dfb8
	addiu	$18,$5,1
	lw	$3,0xc($17)
	addiu	$2,$3,-1
	sltiu	$2,$2,0x43
	beqz	$2,loc_0013df88
	lw	$19,0x2c($17)
	sll	$2,$3,2
	addu	$3,$2,$22
	lw	$16,0($3)
	bne	$16,$17,loc_0013df34
	nop	
	lw	$2,0x3c($17)
	b	loc_0013df88
	sw	$2,0($3)
loc_0013df34:
	beqz	$16,loc_0013df88
	addiu	$18,$5,1
	lw	$3,0x3c($16)
	beql	$3,$17,loc_0013df84
	lw	$2,0x3c($17)
loc_0013df48:
	bnel	$16,$0,loc_0013df74
	move	$16,$3
	move	$4,$20
	jal	0x001ad768
	addiu	$5,$0,0x92
	move	$4,$20
	addiu	$5,$0,0x92
	jal	0x00263ff0
	addiu	$6,$21,0x21c8
	lw	$3,0x3c($16)
	move	$16,$3
loc_0013df74:
	lw	$2,0x3c($16)
	bne	$2,$17,loc_0013df48
	move	$3,$2
	lw	$2,0x3c($17)
loc_0013df84:
	sw	$2,0x10($16)
loc_0013df88:
	jal	0x0013ddf8
	move	$4,$17
	beqz	$19,loc_0013dfb4
	sw	$0,0($17)
loc_0013df98:
	jal	0x0013f6b8
	move	$4,$19
	lw	$19,0x2c($17)
	bnez	$19,loc_0013df98
	lw	$6,-0x4c4c($28)
	b	loc_0013dfbc
	move	$5,$18
loc_0013dfb4:
	lw	$6,-0x4c4c($28)
loc_0013dfb8:
	move	$5,$18
loc_0013dfbc:
	sltu	$2,$5,$6
	bnez	$2,loc_0013dee8
	addiu	$2,$0,0x174
loc_0013dfc8:
	ld	$31,0x70($29)
	ld	$22,0x60($29)
	ld	$21,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x13dd88
	addiu	$29,$29,0x80
	andi	$5,$5,0xff
	lui	$2,0x28
	sll	$8,$5,2
	addiu	$2,$2,0x1a70
	sb	$5,0x18($4)
	addu	$3,$8,$2
	sw	$6,0x1c($4)
	lw	$7,0($3)
	bnel	$7,$0,loc_0013e038
	lw	$2,0x1c($7)
	lui	$2,0x28
	sw	$4,0($3)
	addiu	$2,$2,0x1a90
	sw	$0,0x14($4)
	addu	$2,$8,$2
	sw	$0,0x10($4)
	jr	$31
	sw	$4,0($2)
loc_0013e038:
	sltu	$2,$6,$2
	beqz	$2,loc_0013e058
	lui	$2,0x28
	sw	$0,0x14($4)
	sw	$7,0x10($4)
	sw	$4,0($3)
	jr	$31
	sw	$4,0x14($7)
loc_0013e058:
	addiu	$2,$2,0x1a90
	addu	$5,$8,$2
	lw	$3,0($5)
	lw	$2,0x1c($3)
	sltu	$2,$6,$2
	bnel	$2,$0,loc_0013e08c
	move	$5,$7
	sw	$3,0x14($4)
	sw	$0,0x10($4)
	sw	$4,0($5)
	jr	$31
	sw	$4,0x10($3)
loc_0013e088:
	move	$5,$3
loc_0013e08c:
	lw	$3,0x10($5)
	lw	$2,0x1c($3)
	sltu	$2,$6,$2
	nop	
	beqz	$2,loc_0013e088
	nop	
	sw	$5,0x14($4)
	lw	$2,0x10($5)
	sw	$2,0x10($4)
	sw	$4,0x10($5)
	lw	$2,0x10($4)
	jr	$31
	sw	$4,0x14($2)
	andi	$5,$5,0xff
	lui	$2,0x28
	sll	$8,$5,2
	addiu	$2,$2,0x1a70
	sb	$5,0x18($4)
	addu	$3,$8,$2
	sw	$6,0x1c($4)
	lw	$7,0($3)
	bnel	$7,$0,loc_0013e108
	lw	$2,0x1c($7)
	lui	$2,0x28
	sw	$4,0($3)
	addiu	$2,$2,0x1a90
	sw	$0,0x14($4)
	addu	$2,$8,$2
	sw	$0,0x10($4)
	jr	$31
	sw	$4,0($2)
loc_0013e108:
	sltu	$2,$2,$6
	bnez	$2,loc_0013e128
	lui	$2,0x28
	sw	$0,0x14($4)
	sw	$7,0x10($4)
	sw	$4,0($3)
	jr	$31
	sw	$4,0x14($7)
loc_0013e128:
	addiu	$2,$2,0x1a90
	addu	$5,$8,$2
	lw	$3,0($5)
	lw	$2,0x1c($3)
	sltu	$2,$2,$6
	beql	$2,$0,loc_0013e15c
	move	$5,$7
	sw	$3,0x14($4)
	sw	$0,0x10($4)
	sw	$4,0($5)
	jr	$31
	sw	$4,0x10($3)
loc_0013e158:
	move	$5,$3
loc_0013e15c:
	lw	$3,0x10($5)
	lw	$2,0x1c($3)
	sltu	$2,$2,$6
	nop	
	bnez	$2,loc_0013e158
	nop	
	sw	$5,0x14($4)
	lw	$2,0x10($5)
	sw	$2,0x10($4)
	sw	$4,0x10($5)
	lw	$2,0x10($4)
	jr	$31
	sw	$4,0x14($2)
	.set	macro
	.set	reorder
	.end	isysGObjRemoveAll
