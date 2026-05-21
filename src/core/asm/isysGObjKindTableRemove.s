	.text
	.p2align 3
	.globl	isysGObjKindTableRemove
	.ent	isysGObjKindTableRemove
isysGObjKindTableRemove:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x50
	sd	$17,0x10($29)
	sd	$31,0x40($29)
	move	$17,$4
	sd	$19,0x30($29)
	sd	$18,0x20($29)
	sd	$16,0($29)
	lw	$3,0xc($17)
	addiu	$2,$3,-1
	sltiu	$2,$2,0x43
	beqz	$2,loc_0013e7d8
	lui	$2,0x6b
	sll	$3,$3,2
	addiu	$2,$2,-0x6c30
	addu	$3,$3,$2
	lw	$16,0($3)
	bne	$16,$17,loc_0013e77c
	nop	
	lw	$2,0x3c($16)
	b	loc_0013e7d8
	sw	$2,0($3)
loc_0013e77c:
	beqz	$16,loc_0013e7dc
	ld	$31,0x40($29)
	lw	$3,0x3c($16)
	beq	$3,$17,loc_0013e7d0
	lui	$2,0x55
	lui	$19,0x63
	addiu	$18,$2,0x7a10
loc_0013e798:
	bnel	$16,$0,loc_0013e7c4
	move	$16,$3
	move	$4,$18
	jal	0x001ad768
	addiu	$5,$0,0x92
	move	$4,$18
	addiu	$5,$0,0x92
	jal	0x00263ff0
	addiu	$6,$19,0x21c8
	lw	$3,0x3c($16)
	move	$16,$3
loc_0013e7c4:
	lw	$2,0x3c($16)
	bne	$2,$17,loc_0013e798
	move	$3,$2
loc_0013e7d0:
	lw	$2,0x3c($17)
	sw	$2,0x10($16)
loc_0013e7d8:
	ld	$31,0x40($29)
loc_0013e7dc:
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x50
	nop	
	.set	macro
	.set	reorder
	.end	isysGObjKindTableRemove
