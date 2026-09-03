	.text
	.p2align 3
	.globl	boyAI_sub_143B58
	.ent	boyAI_sub_143B58
boyAI_sub_143B58:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x001007a0
	move	$4,$0
	addiu	$2,$0,-1
	ld	$31,0x40($29)
	slt	$3,$2,$16
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	movn	$2,$0,$3
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x50
	nop	
	jr	$31
	lw	$2,-0x66dc($28)
	jr	$31
	lw	$2,-0x66e8($28)
	addiu	$29,$29,-0x60
	ld	$3,-0x4c18($28)
	sd	$20,0x40($29)
	sd	$19,0x30($29)
	move	$20,$5
	sd	$18,0x20($29)
	move	$19,$4
	sd	$17,0x10($29)
	addiu	$18,$0,1
	sd	$31,0x50($29)
	move	$17,$0
	sd	$16,0($29)
	dsllv	$5,$18,$17
	nop	
loc_00143bd0:
	and	$2,$3,$5
	beql	$2,$0,loc_00143c1c
	ld	$2,0x18($19)
	addiu	$17,$17,1
	sltiu	$2,$17,0x40
	bnel	$2,$0,loc_00143bd0
	dsllv	$5,$18,$17
	lui	$16,0x55
	addiu	$5,$0,0x1f4
	addiu	$16,$16,0x7cc8
	jal	0x001ad768
	move	$4,$16
	lui	$6,0x63
	move	$4,$16
	addiu	$6,$6,0x2220
	jal	0x00263ff0
	addiu	$5,$0,0x1f4
	dsllv	$5,$18,$17
	ld	$2,0x18($19)
loc_00143c1c:
	slti	$4,$17,5
	ld	$3,-0x4c18($28)
	or	$2,$2,$5
	sd	$2,0x18($19)
	or	$3,$3,$5
	bnez	$4,loc_00143c6c
	sd	$3,-0x4c18($28)
	lui	$4,0x55
	jal	0x001a6e28
	addiu	$4,$4,0x7d60
	lui	$16,0x55
	addiu	$5,$0,0x1f8
	addiu	$16,$16,0x7cc8
	jal	0x001ad768
	move	$4,$16
	lui	$6,0x63
	move	$4,$16
	addiu	$6,$6,0x2220
	jal	0x00263ff0
	addiu	$5,$0,0x1f8
loc_00143c6c:
	bgez	$17,loc_00143c84
	sw	$17,0($20)
	lui	$2,0x1d
	sll	$3,$17,0xe
	b	loc_00143c8c
	ori	$2,$2,0x9020
loc_00143c84:
	sll	$3,$17,0xe
	lui	$2,0x1e
loc_00143c8c:
	addu	$2,$3,$2
	ld	$31,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x60
	ld	$2,0x18($4)
	ld	$3,-0x4c18($28)
	nor	$2,$0,$2
	and	$3,$3,$2
	sd	$3,-0x4c18($28)
	jr	$31
	sd	$0,0x18($4)
	nop	
	lui	$2,0x6b
	lw	$4,0($4)
	addiu	$5,$2,-0x6a50
	addiu	$7,$5,0x300
	move	$6,$5
	nop	
loc_00143ce8:
	lw	$2,0($5)
	beq	$2,$4,loc_00143d0c
	move	$3,$6
	addiu	$5,$5,0x30
	slt	$2,$5,$7
	bnez	$2,loc_00143ce8
	addiu	$6,$3,0x30
	jr	$31
	move	$2,$0
loc_00143d0c:
	jr	$31
	move	$2,$6
	nop	
	addiu	$29,$29,-0x80
	sd	$19,0x40($29)
	sd	$18,0x30($29)
	move	$19,$5
	move	$18,$4
	sll	$2,$19,0x10
	andi	$3,$18,0xffff
	lui	$4,0x6b
	or	$3,$3,$2
	addiu	$4,$4,-0x6a50
	sd	$21,0x60($29)
	move	$8,$3
	sd	$20,0x50($29)
	addiu	$9,$4,0x300
	sd	$31,0x70($29)
	move	$21,$6
	sd	$17,0x20($29)
	move	$20,$7
	sd	$16,0x10($29)
	move	$5,$4
	sw	$3,0($29)
	nop	
loc_00143d70:
	lw	$2,0($4)
	beq	$2,$8,loc_00143d94
	move	$3,$5
	addiu	$4,$4,0x30
	slt	$2,$4,$9
	bnez	$2,loc_00143d70
	addiu	$5,$3,0x30
	b	loc_00143d98
	move	$17,$0
loc_00143d94:
	move	$17,$5
loc_00143d98:
	bnez	$17,loc_00143e3c
	move	$2,$17
	lui	$2,0x6b
	sw	$0,0($29)
	addiu	$5,$2,-0x6a50
	move	$7,$0
	addiu	$6,$5,0x300
	move	$4,$5
loc_00143db8:
	lw	$2,0($5)
	beq	$2,$7,loc_00143ddc
	move	$3,$4
	addiu	$5,$5,0x30
	slt	$2,$5,$6
	bnez	$2,loc_00143db8
	addiu	$4,$3,0x30
	b	loc_00143de0
	move	$17,$0
loc_00143ddc:
	move	$17,$4
loc_00143de0:
	bnez	$17,loc_00143e14
	move	$4,$17
	lui	$16,0x55
	addiu	$5,$0,0x14e
	addiu	$16,$16,0x7cc8
	jal	0x001ad768
	move	$4,$16
	lui	$6,0x63
	move	$4,$16
	addiu	$6,$6,0x2220
	jal	0x00263ff0
	addiu	$5,$0,0x14e
	move	$4,$17
loc_00143e14:
	move	$5,$0
	jal	0x002641d8
	addiu	$6,$0,0x30
	addiu	$3,$0,-1
	sh	$18,0($17)
	sh	$19,2($17)
	sh	$20,6($17)
	sh	$21,4($17)
	sw	$3,0x28($17)
	move	$2,$17
loc_00143e3c:
	ld	$31,0x70($29)
	ld	$21,0x60($29)
	ld	$20,0x50($29)
	ld	$19,0x40($29)
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_143B58
