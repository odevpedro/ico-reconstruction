	.text
	.p2align 3
	.globl	girlBrain_sub_16E63C
	.ent	girlBrain_sub_16E63C
girlBrain_sub_16E63C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$3,0x30($7)
	beq	$3,$2,loc_0016e658
	addiu	$2,$0,3
	beq	$3,$2,loc_0016e668
	lui	$3,0x27
	b	loc_0016e67c
	addiu	$5,$0,0xa
loc_0016e658:
	lw	$9,0xf0($29)
	addiu	$9,$9,1
	b	loc_0016e674
	sw	$9,0xf0($29)
loc_0016e668:
	lw	$2,0xf0($29)
	addiu	$2,$2,2
	sw	$2,0xf0($29)
loc_0016e674:
	lui	$3,0x27
	addiu	$5,$0,0xa
loc_0016e67c:
	lw	$2,0x4ec0($3)
	addiu	$6,$3,0x4ec0
	lw	$4,4($6)
	addiu	$3,$0,0x3c
	mult	$2,$2,$5
	beql	$4,$0,loc_0016e698
	break	0,7
loc_0016e698:
	subu	$3,$3,$2
	div	$0,$3,$4
	mflo	$3
	sll	$2,$3,4
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E63C
