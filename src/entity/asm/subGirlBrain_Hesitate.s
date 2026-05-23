	.text
	.p2align 3
	.globl	subGirlBrain_Hesitate
	.ent	subGirlBrain_Hesitate
subGirlBrain_Hesitate:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0xa0
	lui	$2,0x28
	sw	$4,0($29)
	sd	$23,0x80($29)
	sd	$22,0x70($29)
	sd	$21,0x60($29)
	addiu	$22,$2,0x2ac0
	sd	$20,0x50($29)
	addiu	$21,$0,0xa
	sd	$19,0x40($29)
	addiu	$20,$0,0x3c
	sd	$18,0x30($29)
	addiu	$19,$0,1
	sd	$16,0x10($29)
	lui	$18,0x27
	sd	$31,0x90($29)
	addiu	$23,$18,0x4ec0
	lw	$3,0($29)
	addiu	$16,$0,1
	sd	$17,0x20($29)
	lw	$17,0x164($3)
	lw	$2,0x4ec0($18)
loc_00175d38:
	addiu	$4,$0,1
	lw	$5,4($23)
	mult	$2,$2,$21
	sw	$0,0x33c($17)
	beql	$5,$0,loc_00175d50
	break	0,7
loc_00175d50:
	subu	$2,$20,$2
	div	$0,$2,$5
	mflo	$2
	srl	$3,$2,0x1f
	addu	$2,$2,$3
	sra	$2,$2,1
	div	$0,$16,$2
	mfhi	$3
	bnez	$3,loc_00175d7c
	addiu	$16,$16,1
	sb	$19,0x58e0($22)
loc_00175d7c:
	jal	0x00203aa0
	nop	
	b	loc_00175d38
	lw	$2,0x4ec0($18)
	nop	
	.set	macro
	.set	reorder
	.end	subGirlBrain_Hesitate
