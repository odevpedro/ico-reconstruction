/* 0x0013F3F0: core process registration body.
 * The branch at 0x0013F434 is emitted as raw bytes to preserve the
 * original jump encoding.
 */
	.text
	.p2align 3
	.globl	isysGObjProcAdd_
	.ent	isysGObjProcAdd_
isysGObjProcAdd_:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x90
	sd	$23,0x70($29)
	sd	$22,0x60($29)
	move	$23,$9
	sd	$21,0x50($29)
	andi	$22,$7,0xff
	sd	$20,0x40($29)
	move	$21,$5
	sd	$19,0x30($29)
	move	$20,$6
	sd	$18,0x20($29)
	move	$19,$8
	sd	$31,0x80($29)
	move	$18,$4
	sd	$17,0x10($29)
	bnez	$20,0f
	sd	$16,0($29)
	.word	0x10000074  # j 0x0013F608
	move	$2,$0
0:
	lw	$5,-0x4c44($28)
	beqz	$5,2f
	move	$4,$0
	lw	$3,-0x4c48($28)
	lw	$2,0($3)
	beqz	$2,2f
	nop	
	move	$6,$5
	addiu	$4,$4,1
1:
	sltu	$2,$4,$6
	beqz	$2,2f
	addiu	$3,$3,0x94
	lw	$2,0($3)
	bnel	$2,$0,1b
	addiu	$4,$4,1
2:
	bne	$4,$5,5f
	addiu	$2,$0,0x94
	lui	$16,0x55
	move	$17,$0
	addiu	$16,$16,0x7ae8
	jal	0x001a6e28
	move	$4,$16
	jal	0x001a6e28
	move	$4,$16
	lw	$2,-0x4c44($28)
	beqz	$2,4f
	lui	$16,0x55
	addiu	$2,$0,0x94
	nop	
3:
	lw	$3,-0x4c48($28)
	mult	$2,$17,$2
	addiu	$4,$16,0x7b10
	addiu	$17,$17,1
	addu	$2,$2,$3
	lw	$7,0x5c($2)
	lw	$5,0($2)
	jal	0x001a6e28
	lw	$6,0x1c($2)
	lw	$2,-0x4c44($28)
	sltu	$2,$17,$2
	bnez	$2,3b
	addiu	$2,$0,0x94
4:
	b	6f
	move	$17,$0
5:
	lw	$3,-0x4c48($28)
	mult	$2,$4,$2
	addu	$17,$3,$2
6:
	bnez	$17,7f
	lui	$4,0x55
	jal	0x001a6e28
	addiu	$4,$4,0x7b20
	b	5f
	move	$2,$0
7:
	bnez	$22,8f
	sw	$17,0($17)
	addiu	$16,$17,0x24
	lw	$8,-0x68e8($28)
	move	$7,$17
	move	$6,$20
	movn	$7,$21,$21
	move	$9,$23
	move	$4,$16
	addiu	$5,$0,1
	jal	0x0013d1b0
	move	$10,$19
	jal	0x0013d3c8
	move	$4,$16
	b	9f
	sw	$0,0x1c($17)
8:
	sw	$20,0x1c($17)
9:
	addiu	$2,$0,1
	lw	$4,0x2c($18)
	sw	$22,0x10($17)
	sw	$2,0x18($17)
	sw	$18,4($17)
	bnez	$4,0f
	sw	$19,0x14($17)
	sw	$0,0xc($17)
	sw	$0,8($17)
	sw	$17,0x30($18)
	b	4f
	sw	$17,0x2c($18)
0:
	lw	$2,0x14($4)
	sltu	$2,$19,$2
	beql	$2,$0,1f
	lw	$3,0x30($18)
	sw	$0,0xc($17)
	lw	$2,0x2c($18)
	sw	$2,8($17)
	sw	$17,0xc($2)
	b	4f
	sw	$17,0x2c($18)
1:
	lw	$2,0x14($3)
	sltu	$2,$19,$2
	bnel	$2,$0,3f
	lw	$3,8($4)
	sw	$3,0xc($17)
	sw	$0,8($17)
	sw	$17,8($3)
	b	4f
	sw	$17,0x30($18)
2:
	move	$4,$3
	lw	$3,8($4)
3:
	lw	$2,0x14($3)
	sltu	$2,$19,$2
	nop	
	beqz	$2,2b
	nop	
	sw	$4,0xc($17)
	lw	$2,8($4)
	sw	$2,8($17)
	sw	$17,8($4)
	lw	$2,8($17)
	sw	$17,0xc($2)
4:
	move	$2,$17
5:
	ld	$31,0x80($29)
	ld	$23,0x70($29)
	ld	$22,0x60($29)
	ld	$21,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	isysGObjProcAdd_
