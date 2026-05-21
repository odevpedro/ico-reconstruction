	.text
	.p2align 3
	.globl	isysGObjAddBeforeGObj
	.ent	isysGObjAddBeforeGObj
isysGObjAddBeforeGObj:
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
	beqz	$6,loc_0013e3a8
	move	$4,$0
	lw	$3,-0x4c50($28)
	lw	$2,0($3)
	beqz	$2,loc_0013e3a8
	nop	
	move	$5,$6
	addiu	$4,$4,1
	nop	
loc_0013e390:
	sltu	$2,$4,$5
	beqz	$2,loc_0013e3a8
	addiu	$3,$3,0x174
	lw	$2,0($3)
	bnel	$2,$0,loc_0013e390
	addiu	$4,$4,1
loc_0013e3a8:
	bne	$4,$6,loc_0013e3c4
	addiu	$2,$0,0x174
	lui	$4,0x55
	jal	0x001a6e28
	addiu	$4,$4,0x7a30
	b	loc_0013e3dc
	move	$4,$0
loc_0013e3c4:
	lw	$3,-0x4c50($28)
	mult	$2,$4,$2
	addu	$2,$2,$3
	sw	$0,0x164($2)
	move	$4,$2
	sw	$0,0x170($2)
loc_0013e3dc:
	bnez	$4,loc_0013e3f0
	nop	
	lui	$4,0x55
	b	loc_0013e400
	addiu	$4,$4,0x7a30
loc_0013e3f0:
	bnel	$16,$0,loc_0013e410
	sw	$4,0($4)
	lui	$4,0x55
	addiu	$4,$4,0x7a20
loc_0013e400:
	jal	0x001a6e28
	nop	
	b	loc_0013e474
	move	$2,$0
loc_0013e410:
	sw	$17,0x28($4)
	lbu	$2,0x18($16)
	sb	$2,0x18($4)
	lw	$3,0x14($16)
	sw	$16,0x10($4)
	sw	$3,0x14($4)
	sw	$4,0x14($16)
	lw	$2,0x1c($16)
	lw	$3,0x14($4)
	bnez	$3,loc_0013e454
	sw	$2,0x1c($4)
	lbu	$3,0x18($4)
	lui	$2,0x28
	addiu	$2,$2,0x1a70
	sll	$3,$3,2
	addu	$3,$3,$2
	sw	$4,0($3)
loc_0013e454:
	addiu	$3,$0,-1
	sw	$0,0x15c($4)
	sw	$3,4($4)
	move	$2,$4
	sw	$3,8($4)
	sw	$0,0x2c($4)
	sw	$0,0x30($4)
	sw	$0,0x58($4)
loc_0013e474:
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	lw	$9,-0x4c4c($28)
	move	$7,$0
	beqz	$9,loc_0013e4c4
	move	$6,$0
	lw	$8,-0x4c50($28)
	nop	
loc_0013e4a0:
	addiu	$3,$0,0x174
	addiu	$5,$6,1
	mult	$2,$7,$3
	addiu	$7,$7,1
	sltu	$4,$7,$9
	addu	$3,$2,$8
	lw	$2,0($3)
	bnez	$4,loc_0013e4a0
	movn	$6,$5,$2
loc_0013e4c4:
	jr	$31
	move	$2,$6
	nop	
	.set	macro
	.set	reorder
	.end	isysGObjAddBeforeGObj
