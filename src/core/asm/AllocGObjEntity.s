	.text
	.p2align 3
	.globl	AllocGObjEntity
	.ent	AllocGObjEntity
AllocGObjEntity:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x50
	addiu	$7,$0,0x1ce
	sd	$17,0x10($29)
	sd	$19,0x30($29)
	move	$17,$4
	sd	$18,0x20($29)
	lui	$19,0x61
	move	$18,$5
	lw	$4,-0x68cc($28)
	sd	$16,0($29)
	addiu	$5,$0,0x850
	sd	$31,0x40($29)
	jal	0x0013a0f8
	addiu	$6,$19,0x2620
	move	$16,$2
	lui	$2,0x2f
	move	$3,$16
	addiu	$4,$2,0x23f0
	addiu	$2,$4,0x840
loc_0019f35c:
	ld	$5,0($4)
	ld	$6,8($4)
	ld	$7,0x10($4)
	ld	$8,0x18($4)
	sd	$5,0($3)
	sd	$6,8($3)
	sd	$7,0x10($3)
	sd	$8,0x18($3)
	addiu	$4,$4,0x20
	addiu	$3,$3,0x20
	nop	
	bne	$4,$2,loc_0019f35c
	nop	
	ld	$5,0($4)
	ld	$6,8($4)
	sd	$5,0($3)
	addiu	$2,$0,0x5eb
	beq	$17,$2,loc_0019f3b4
	sd	$6,8($3)
	move	$5,$17
	jal	0x001a27f8
	move	$4,$16
loc_0019f3b4:
	lui	$4,0x61
	jal	0x001a6e28
	addiu	$4,$4,0x2640
	lw	$2,0x820($16)
	bnel	$2,$0,loc_0019f3dc
	move	$4,$16
	lw	$2,0x8c($16)
	beql	$2,$0,loc_0019f3e8
	lui	$4,0x61
	move	$4,$16
loc_0019f3dc:
	jal	0x0019ef58
	move	$5,$18
	lui	$4,0x61
loc_0019f3e8:
	jal	0x001a6e28
	addiu	$4,$4,0x2658
	move	$5,$18
	jal	0x0019e648
	move	$4,$16
	lui	$4,0x61
	jal	0x001a6e28
	addiu	$4,$4,0x2678
	lw	$2,0x8c($16)
	beqz	$2,loc_0019f4b8
	lui	$4,0x61
	jal	0x0019ef10
	move	$4,$16
	lw	$4,-0x68e0($28)
	addiu	$6,$19,0x2620
	addiu	$5,$0,0x35
	jal	0x0013a0f8
	addiu	$7,$0,0x1b7
	sw	$2,0x810($16)
	addiu	$10,$0,-1
	move	$8,$0
	lw	$5,0x8c($16)
loc_0019f440:
	lw	$2,0($5)
	beq	$2,$10,loc_0019f490
	move	$6,$0
	lw	$9,0x810($16)
	move	$4,$0
	addiu	$7,$8,1
	addu	$2,$4,$5
	nop	
loc_0019f460:
	lw	$3,4($2)
	beq	$3,$8,loc_0019f49c
	move	$2,$6
	addiu	$6,$6,1
	sll	$2,$6,6
	move	$4,$2
	addu	$3,$2,$5
	lw	$2,0($3)
	bne	$2,$10,loc_0019f460
	addu	$2,$4,$5
	b	loc_0019f49c
	addiu	$2,$0,-1
loc_0019f490:
	lw	$9,0x810($16)
	addiu	$7,$8,1
	addiu	$2,$0,-1
loc_0019f49c:
	addu	$3,$9,$8
	move	$8,$7
	sb	$2,0($3)
	slti	$2,$8,0x35
	bnel	$2,$0,loc_0019f440
	lw	$5,0x8c($16)
	lui	$4,0x61
loc_0019f4b8:
	jal	0x001a6e28
	addiu	$4,$4,0x2698
	move	$2,$16
	ld	$31,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x50
	.set	macro
	.set	reorder
	.end	AllocGObjEntity
