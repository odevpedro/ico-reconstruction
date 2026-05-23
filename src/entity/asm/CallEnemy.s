	.text
	.p2align 3
	.globl	CallEnemy
	.ent	CallEnemy
CallEnemy:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0xb0
	sd	$22,0x70($29)
	sd	$30,0x90($29)
	move	$22,$4
	sd	$31,0xa0($29)
	move	$30,$5
	sd	$23,0x80($29)
	addiu	$4,$0,4
	sd	$21,0x60($29)
	sd	$20,0x50($29)
	sd	$19,0x40($29)
	sd	$18,0x30($29)
	sd	$17,0x20($29)
	sd	$16,0x10($29)
	sw	$6,0($29)
	jal	0x0013eb50
	sw	$7,4($29)
	move	$6,$2
	beqz	$6,loc_00192a18
	move	$21,$0
	lui	$16,0x2a
	lw	$5,8($6)
loc_00192960:
	addiu	$2,$0,0x4c
	addiu	$3,$16,0x4c48
	lw	$8,0x164($6)
	mult	$2,$5,$2
	beqz	$22,loc_00192994
	addu	$7,$2,$3
	lw	$3,8($22)
	addiu	$2,$0,0xd7e
	bne	$3,$2,loc_00192998
	addiu	$2,$0,0x4c
	addiu	$2,$0,0xd7d
	bne	$5,$2,loc_00192a00
	nop	
loc_00192994:
	addiu	$2,$0,0x4c
loc_00192998:
	addiu	$4,$16,0x4c48
	mult	$2,$5,$2
	addu	$2,$2,$4
	lhu	$3,0x44($2)
	beql	$3,$0,loc_001929c8
	ld	$2,0x18($8)
	beql	$22,$0,loc_001929c8
	ld	$2,0x18($8)
	lw	$2,8($22)
	bne	$3,$2,loc_00192a00
	nop	
	ld	$2,0x18($8)
loc_001929c8:
	dsrl32	$2,$2,2
	andi	$2,$2,1
	bnez	$2,loc_00192a00
	nop	
	lw	$2,0x48($7)
	srl	$2,$2,0x15
	andi	$2,$2,1
	bnez	$2,loc_00192a00
	addiu	$2,$0,-1
	lh	$3,0x42($7)
	beq	$3,$2,loc_00192a18
	move	$21,$6
	bgtz	$3,loc_00192a18
	nop	
loc_00192a00:
	jal	0x0013ebe0
	move	$4,$6
	move	$6,$2
	bnel	$6,$0,loc_00192960
	lw	$5,8($6)
	move	$21,$0
loc_00192a18:
	.word	0x12a00041
	addiu	$2,$0,0x4c
	lw	$5,8($21)
	lui	$4,0x2a
	lui	$6,0x20
	.set	macro
	.set	reorder
	.end	CallEnemy
