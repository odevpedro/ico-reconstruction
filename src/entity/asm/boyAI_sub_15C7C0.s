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
	.word	0x1471000e
	addiu	$6,$0,9
	addiu	$5,$0,1
	sw	$5,0xc($29)
	addiu	$5,$2,0x6c
loc_0015c8a0:
	addiu	$6,$6,1
	slti	$2,$6,0xa
	.word	0x10400007
	addiu	$5,$5,0xc
	lw	$2,0($5)
	.word	0x14510004
	lw	$9,0xc($29)
	addiu	$9,$9,1
	b	loc_0015c8a0
	sw	$9,0xc($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15C7C0
