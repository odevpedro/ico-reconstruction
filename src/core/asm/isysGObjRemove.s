	.text
	.p2align 3
	.globl	isysGObjRemove
	.ent	isysGObjRemove
isysGObjRemove:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x60
	sd	$17,0x10($29)
	sd	$31,0x50($29)
	move	$17,$4
	sd	$20,0x40($29)
	sd	$19,0x30($29)
	sd	$18,0x20($29)
	sd	$16,0($29)
	lw	$3,0xc($17)
	addiu	$2,$3,-1
	sltiu	$2,$2,0x43
	beqz	$2,loc_0013e600
	lw	$18,0x2c($17)
	lui	$2,0x6b
	sll	$3,$3,2
	addiu	$2,$2,-0x6c30
	addu	$3,$3,$2
	lw	$16,0($3)
	bne	$16,$17,loc_0013e5a4
	nop	
	lw	$2,0x3c($17)
	b	loc_0013e600
	sw	$2,0($3)
loc_0013e5a4:
	beqz	$16,loc_0013e600
	nop	
	lw	$3,0x3c($16)
	beq	$3,$17,loc_0013e5f8
	lui	$2,0x55
	lui	$20,0x63
	addiu	$19,$2,0x7a10
loc_0013e5c0:
	bnel	$16,$0,loc_0013e5ec
	move	$16,$3
	move	$4,$19
	jal	0x001ad768
	addiu	$5,$0,0x92
	move	$4,$19
	addiu	$5,$0,0x92
	jal	0x00263ff0
	addiu	$6,$20,0x21c8
	lw	$3,0x3c($16)
	move	$16,$3
loc_0013e5ec:
	lw	$2,0x3c($16)
	bne	$2,$17,loc_0013e5c0
	move	$3,$2
loc_0013e5f8:
	lw	$2,0x3c($17)
	sw	$2,0x10($16)
loc_0013e600:
	jal	0x0013ddf8
	move	$4,$17
	beqz	$18,loc_0013e624
	sw	$0,0($17)
loc_0013e610:
	jal	0x0013f6b8
	move	$4,$18
	lw	$18,0x2c($17)
	bnez	$18,loc_0013e610
	nop	
loc_0013e624:
	ld	$31,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x60
	nop	
	.set	macro
	.set	reorder
	.end	isysGObjRemove
