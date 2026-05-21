	.text
	.p2align 3
	.globl	isysGObjAddAfterGObj
	.ent	isysGObjAddAfterGObj
isysGObjAddAfterGObj:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x30
	lw	$6,-0x4c4c($28)
	sd	$17,0x10($29)
	sd	$16,0($29)
	move	$17,$4
	sd	$31,0x20($29)
	move	$16,$5
	beqz	$6,loc_0013e270
	move	$4,$0
	lw	$3,-0x4c50($28)
	lw	$2,0($3)
	beqz	$2,loc_0013e270
	move	$5,$6
	addiu	$4,$4,1
loc_0013e258:
	sltu	$2,$4,$5
	beqz	$2,loc_0013e270
	addiu	$3,$3,0x174
	lw	$2,0($3)
	bnel	$2,$0,loc_0013e258
	addiu	$4,$4,1
loc_0013e270:
	bne	$4,$6,loc_0013e28c
	addiu	$2,$0,0x174
	lui	$4,0x55
	jal	0x001a6e28
	addiu	$4,$4,0x7a30
	b	loc_0013e2a4
	move	$5,$0
loc_0013e28c:
	lw	$3,-0x4c50($28)
	mult	$2,$4,$2
	addu	$2,$2,$3
	sw	$0,0x164($2)
	move	$5,$2
	sw	$0,0x170($2)
loc_0013e2a4:
	bnez	$5,loc_0013e2b4
	lui	$4,0x55
	b	loc_0013e2c4
	addiu	$4,$4,0x7a30
loc_0013e2b4:
	bnel	$16,$0,loc_0013e2d4
	sw	$5,0($5)
	lui	$4,0x55
	addiu	$4,$4,0x7a20
loc_0013e2c4:
	jal	0x001a6e28
	nop	
	b	loc_0013e33c
	move	$2,$0
loc_0013e2d4:
	sw	$17,0x28($5)
	lbu	$2,0x18($16)
	sw	$16,0x14($5)
	sb	$2,0x18($5)
	lw	$3,0x10($16)
	lw	$4,0x1c($16)
	sw	$3,0x10($5)
	sw	$5,0x10($16)
	sw	$4,0x1c($5)
	lw	$2,0x10($5)
	bnez	$2,loc_0013e320
	addiu	$3,$0,-1
	lbu	$3,0x18($5)
	lui	$2,0x28
	addiu	$2,$2,0x1a90
	sll	$3,$3,2
	addu	$3,$3,$2
	sw	$5,0($3)
	addiu	$3,$0,-1
loc_0013e320:
	sw	$0,0x15c($5)
	sw	$3,4($5)
	move	$2,$5
	sw	$3,8($5)
	sw	$0,0x2c($5)
	sw	$0,0x30($5)
	sw	$0,0x58($5)
loc_0013e33c:
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	.set	macro
	.set	reorder
	.end	isysGObjAddAfterGObj
