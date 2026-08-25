	.text
	.p2align 3
	.globl	boyAI_sub_15C878
	.ent	boyAI_sub_15C878
boyAI_sub_15C878:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x1000fff7
	sw	$4,8($29)
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
	.word	0x14400017
	nop	
loc_0015c950:
	lw	$2,0($29)
	lw	$3,0x164($2)
	lw	$4,0x678($3)
	lw	$2,0x38c($4)
	.word	0x1c400011
	nop	
	ld	$2,0x20($19)
	dsll	$2,$2,5
	dsra32	$2,$2,0
	andi	$2,$2,1
	.word	0x1440000b
	addiu	$2,$0,1
	lw	$3,0x30($19)
	beq	$3,$2,loc_0015c994
	addiu	$2,$0,0x45
	.word	0x546200f4
	sw	$0,4($29)
loc_0015c994:
	ld	$2,0x18($19)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15C878
