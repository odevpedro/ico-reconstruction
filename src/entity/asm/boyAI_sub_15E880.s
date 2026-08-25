	.text
	.p2align 3
	.globl	boyAI_sub_15E880
	.ent	boyAI_sub_15E880
boyAI_sub_15E880:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$19,0x40($29)
	sd	$18,0x30($29)
	addiu	$19,$0,1
	sd	$16,0x10($29)
	addiu	$18,$0,2
	sd	$31,0x50($29)
	lui	$16,0xffff
	lw	$2,0($29)
	ori	$16,$16,0xffff
	sd	$17,0x20($29)
	lw	$17,0x164($2)
	nop	
loc_0015e8b0:
	lw	$2,0($29)
	sw	$16,0x34($17)
	lw	$3,0x164($2)
	lw	$4,0x670($3)
	lw	$2,0x294($4)
	beq	$2,$19,loc_0015e8dc
	addiu	$5,$0,0x81
	beq	$2,$18,loc_0015e8e8
	nop	
	b	loc_0015e8f4
	nop	
loc_0015e8dc:
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
loc_0015e8e8:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x82
loc_0015e8f4:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015e8b0
	nop	
	nop	
	addiu	$29,$29,-0x30
	sw	$4,0($29)
	sd	$31,0x20($29)
	lw	$2,0($29)
	sd	$16,0x10($29)
	lw	$16,0x164($2)
	ld	$2,0x470($16)
	nop	
	addiu	$5,$0,0xb7
	dsll	$3,$2,0x16
	dsra32	$3,$3,0
	dsll	$2,$2,0x17
	dsra32	$2,$2,0
	.set	macro
	.set	reorder
	.end	boyAI_sub_15E880
