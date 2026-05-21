	.text
	.p2align 3
	.globl	isysGObjKindTableAdd
	.ent	isysGObjKindTableAdd
isysGObjKindTableAdd:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x50
	lw	$2,-0x6730($28)
	sd	$18,0x20($29)
	sd	$17,0x10($29)
	move	$18,$5
	sd	$31,0x40($29)
	move	$17,$4
	sd	$19,0x30($29)
	beqz	$2,loc_0013e678
	sd	$16,0($29)
	b	loc_0013e70c
	sw	$18,0xc($17)
loc_0013e678:
	jal	0x0013eb50
	lw	$4,0xc($17)
	move	$16,$2
	b	loc_0013e698
	sltiu	$19,$18,0x44
	nop	
loc_0013e690:
	jal	0x0013ebe0
	move	$4,$16
loc_0013e698:
	beqz	$16,loc_0013e6b0
	nop	
	bne	$16,$17,loc_0013e690
	nop	
	jal	0x0013e728
	move	$4,$17
loc_0013e6b0:
	beqz	$19,loc_0013e70c
	sw	$18,0xc($17)
	lui	$2,0x6b
	sll	$3,$18,2
	addiu	$2,$2,-0x6c30
	addu	$3,$3,$2
	lw	$2,0($3)
	bnez	$2,loc_0013e6dc
	move	$16,$2
	b	loc_0013e708
	sw	$17,0($3)
loc_0013e6dc:
	lw	$3,0x3c($16)
	beql	$3,$0,loc_0013e708
	sw	$17,0x3c($16)
loc_0013e6e8:
	move	$16,$3
	lw	$2,0x3c($16)
	move	$3,$2
	nop	
	nop	
	bnez	$2,loc_0013e6e8
	nop	
	sw	$17,0x3c($16)
loc_0013e708:
	sw	$0,0x3c($17)
loc_0013e70c:
	ld	$31,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x50
	.set	macro
	.set	reorder
	.end	isysGObjKindTableAdd
