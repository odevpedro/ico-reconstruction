	.text
	.p2align 3
	.globl	boyAI_sub_15C7C0
	.ent	boyAI_sub_15C7C0
boyAI_sub_15C7C0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	nop	
	addiu	$29,$29,-0xd0
	addiu	$2,$0,0x14
	sw	$4,0($29)
	lui	$9,0x56
	sd	$18,0x50($29)
	lui	$4,0x29
	sd	$31,0xc0($29)
	addiu	$4,$4,-0x3230
	sd	$30,0xb0($29)
	addiu	$5,$9,0x4b00
	sd	$23,0xa0($29)
	move	$6,$0
	sd	$22,0x90($29)
	sd	$21,0x80($29)
	sd	$20,0x70($29)
	sd	$17,0x40($29)
	sd	$16,0x30($29)
	sw	$0,4($29)
	lw	$3,0($29)
	sd	$19,0x60($29)
	lw	$19,0x164($3)
	sw	$0,8($29)
	sw	$0,0xc($29)
	lw	$8,0x44($19)
	mult	$2,$8,$2
	sll	$18,$8,2
	addu	$5,$18,$5
	lw	$3,0($5)
	addu	$2,$2,$4
	lw	$17,0($2)
	bne	$3,$17,loc_0015c880
	lw	$7,-0x6e0c($28)
	addiu	$2,$0,1
	sw	$2,8($29)
	move	$3,$5
	nop	
loc_0015c858:
	addiu	$6,$6,1
	slti	$2,$6,9
	beqz	$2,loc_0015c880
	addiu	$3,$3,0xc
	lw	$2,0($3)
	bne	$2,$17,loc_0015c880
	lw	$4,8($29)
	addiu	$4,$4,1
	b	loc_0015c858
	sw	$4,8($29)
loc_0015c880:
	addiu	$5,$9,0x4b00
	addu	$2,$18,$5
	lw	$3,0x6c($2)
	bne	$3,$17,loc_0015c8c8
	addiu	$6,$0,9
	addiu	$5,$0,1
	sw	$5,0xc($29)
	addiu	$5,$2,0x6c
loc_0015c8a0:
	addiu	$6,$6,1
	slti	$2,$6,0xa
	beqz	$2,loc_0015c8c8
	addiu	$5,$5,0xc
	lw	$2,0($5)
	bne	$2,$17,loc_0015c8c8
	lw	$9,0xc($29)
	addiu	$9,$9,1
	b	loc_0015c8a0
	sw	$9,0xc($29)
loc_0015c8c8:
	lui	$2,0x56
	addiu	$4,$0,0x14
	addiu	$22,$2,0x4b00
	.word	0x01042018
	addu	$2,$18,$22
	lui	$3,0x29
	sw	$2,0x20($29)
	addiu	$3,$3,-0x322c
	sw	$2,0x24($29)
	addiu	$5,$3,0xc
	lui	$2,0x29
	addu	$6,$4,$3
	addiu	$20,$2,0x2548
	addu	$5,$4,$5
	addiu	$2,$3,4
	sw	$6,0x10($29)
	addu	$2,$4,$2
	addiu	$3,$3,8
	addu	$4,$4,$3
	sw	$5,0x1c($29)
	sw	$2,0x14($29)
	addiu	$30,$0,0x3c
	sw	$4,0x18($29)
	addiu	$23,$0,0xc
	addiu	$21,$0,0x18
	nop	
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$3,0x15c($2)
	bne	$4,$7,loc_0015c950
	lw	$16,0x4a0($3)
	lw	$2,-0x633c($28)
	bnez	$2,loc_0015c9a8
	nop	
loc_0015c950:
	lw	$2,0($29)
	lw	$3,0x164($2)
	lw	$4,0x678($3)
	lw	$2,0x38c($4)
	bgtz	$2,loc_0015c9a8
	nop	
	ld	$2,0x20($19)
	dsll	$2,$2,5
	dsra32	$2,$2,0
	andi	$2,$2,1
	bnez	$2,loc_0015c9a8
	addiu	$2,$0,1
	lw	$3,0x30($19)
	beq	$3,$2,loc_0015c994
	addiu	$2,$0,0x45
	.word	0x546200f4
	sw	$0,4($29)
loc_0015c994:
	ld	$2,0x18($19)
	dsrl32	$2,$2,0x15
	andi	$2,$2,1
	beqz	$2,loc_0015c9c0
	nop	
loc_0015c9a8:
	lw	$4,0($29)
	addiu	$5,$0,0xa7
	jal	0x0015bcc8
	sw	$0,4($29)
	.word	0x100000e9
	nop	
loc_0015c9c0:
	lw	$2,0($29)
	bne	$2,$7,loc_0015c9ec
	lw	$6,8($29)
	jal	0x0014a0d8
	nop	
	beqz	$2,loc_0015c9e8
	addiu	$8,$0,9
	addiu	$7,$0,0xa
	b	loc_0015c9f4
	lw	$6,0xc($29)
loc_0015c9e8:
	lw	$6,8($29)
loc_0015c9ec:
	move	$8,$0
	addiu	$7,$0,9
loc_0015c9f4:
	.word	0x1611008c
	move	$3,$8
	ld	$2,0x20($19)
	lw	$9,4($29)
	dsll	$2,$2,4
	dsra32	$2,$2,0
	addiu	$3,$9,1
	andi	$2,$2,1
	movz	$9,$3,$2
	.word	0x18c00048
	sw	$9,4($29)
	lui	$3,0x27
	addiu	$9,$0,0xa
	lw	$2,0x4ec0($3)
	move	$5,$3
	addiu	$5,$5,0x4ec0
	addiu	$4,$0,0x15e
	mult	$2,$2,$9
	lw	$3,4($5)
	addiu	$5,$6,1
	beql	$3,$0,loc_0015ca4c
	break	0,7
loc_0015ca4c:
	subu	$2,$30,$2
	div	$0,$2,$3
	mflo	$2
	mult	$2,$2,$4
	lw	$4,4($29)
	div	$0,$2,$30
	mflo	$2
	mult	$2,$2,$5
	slt	$2,$2,$4
	.word	0x104000bb
	addu	$5,$8,$6
	slt	$2,$5,$7
	beqz	$2,loc_0015ca9c
	mult	$3,$5,$23
	addiu	$4,$0,0x43e
	addu	$2,$3,$18
	addu	$2,$2,$22
	lw	$3,0($2)
	bne	$3,$4,loc_0015caa4
	mult	$3,$5,$23
loc_0015ca9c:
	move	$5,$8
	mult	$3,$5,$23
loc_0015caa4:
	lw	$7,0x14($29)
	lw	$5,0x10($29)
	lw	$8,0x18($29)
	lw	$4,0($5)
	addu	$2,$3,$18
	lw	$9,0x1c($29)
	addu	$2,$2,$22
	.set	macro
	.set	reorder
	.end	boyAI_sub_15C7C0
