	.text
	.p2align 3
	.globl	boyAI_sub_15896C
	.ent	boyAI_sub_15896C
boyAI_sub_15896C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$18,$0,1
	sd	$16,0($29)
	sd	$31,0x30($29)
	move	$16,$4
	lw	$2,0x164($16)
	lw	$4,0x670($2)
	lw	$3,0x310($4)
	beq	$3,$18,loc_001589cc
	move	$17,$5
	slti	$2,$3,2
	beql	$2,$0,loc_001589ac
	addiu	$2,$0,2
	beqz	$3,loc_001589bc
	ld	$31,0x30($29)
	b	loc_00158a3c
	ld	$18,0x20($29)
loc_001589ac:
	beq	$3,$2,loc_00158a0c
	ld	$31,0x30($29)
	b	loc_00158a3c
	ld	$18,0x20($29)
loc_001589bc:
	bnel	$17,$18,loc_00158a3c
	ld	$18,0x20($29)
	b	loc_00158a20
	nop	
loc_001589cc:
	beqz	$17,loc_001589dc
	addiu	$2,$0,2
	bne	$17,$2,loc_001589f4
	move	$4,$16
loc_001589dc:
	jal	0x00158928
	move	$4,$16
	lw	$2,0x164($16)
	lw	$3,0x670($2)
	b	loc_00158a34
	sw	$17,0x310($3)
loc_001589f4:
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x1588f0
	addiu	$29,$29,0x40
loc_00158a0c:
	beql	$17,$0,loc_00158a14
	sw	$0,0x310($4)
loc_00158a14:
	addiu	$2,$0,3
	bne	$17,$2,loc_00158a38
	ld	$31,0x30($29)
loc_00158a20:
	jal	0x001588f0
	move	$4,$16
	lw	$2,0x164($16)
	lw	$3,0x670($2)
	sw	$18,0x310($3)
loc_00158a34:
	ld	$31,0x30($29)
loc_00158a38:
	ld	$18,0x20($29)
loc_00158a3c:
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x40
	nop	
	addiu	$29,$29,-0x1e0
	lui	$2,0x16
	sw	$4,0($29)
	addiu	$2,$2,-0xab0
	sd	$31,0x1b0($29)
	addiu	$4,$0,0x10
	sd	$30,0x1a0($29)
	sd	$23,0x190($29)
	sd	$22,0x180($29)
	sd	$21,0x170($29)
	sd	$20,0x160($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15896C
