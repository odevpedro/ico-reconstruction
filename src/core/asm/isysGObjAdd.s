	.text
	.p2align 3
	.globl	isysGObjAdd
	.ent	isysGObjAdd
isysGObjAdd:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x50
	lw	$7,-0x4c4c($28)
	sd	$18,0x20($29)
	sd	$19,0x30($29)
	move	$18,$4
	sd	$17,0x10($29)
	move	$19,$6
	sd	$31,0x40($29)
	andi	$17,$5,0xff
	sd	$16,0($29)
	beqz	$7,loc_0013e938
	move	$4,$0
	lw	$3,-0x4c50($28)
	lw	$2,0($3)
	beqz	$2,loc_0013e938
	nop	
	move	$5,$7
	addiu	$4,$4,1
loc_0013e920:
	sltu	$2,$4,$5
	beqz	$2,loc_0013e938
	addiu	$3,$3,0x174
	lw	$2,0($3)
	bnel	$2,$0,loc_0013e920
	addiu	$4,$4,1
loc_0013e938:
	bne	$4,$7,loc_0013e958
	addiu	$2,$0,0x174
	lui	$4,0x55
	move	$16,$0
	jal	0x001a6e28
	addiu	$4,$4,0x7a30
	b	loc_0013e970
	nop	
loc_0013e958:
	lw	$3,-0x4c50($28)
	mult	$2,$4,$2
	addu	$2,$2,$3
	sw	$0,0x164($2)
	move	$16,$2
	sw	$0,0x170($2)
loc_0013e970:
	bnel	$16,$0,loc_0013e98c
	sw	$18,0x28($16)
	lui	$4,0x55
	jal	0x001a6e28
	addiu	$4,$4,0x7a30
	b	loc_0013e9c4
	move	$2,$0
loc_0013e98c:
	move	$5,$17
	sw	$16,0($16)
	move	$6,$19
	jal	0x0013dff0
	move	$4,$16
	addiu	$3,$0,-1
	sw	$0,0x15c($16)
	sw	$3,4($16)
	move	$2,$16
	sw	$3,8($16)
	sw	$0,0x2c($16)
	sw	$0,0x30($16)
	sw	$0,0x58($16)
	sw	$0,0xc($16)
loc_0013e9c4:
	ld	$31,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x50
	.set	macro
	.set	reorder
	.end	isysGObjAdd
