	.text
	.p2align 3
	.globl	subGirlBrain_Busy
	.ent	subGirlBrain_Busy
subGirlBrain_Busy:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0xa0
	lui	$2,0x28
	sw	$4,0($29)
	sd	$23,0x80($29)
	sd	$22,0x70($29)
	addiu	$23,$0,1
	sd	$21,0x60($29)
	lui	$22,0x27
	sd	$20,0x50($29)
	move	$21,$22
	sd	$16,0x10($29)
	addiu	$20,$2,0x2ac0
	sd	$31,0x90($29)
	move	$16,$0
	sd	$19,0x40($29)
	sd	$18,0x30($29)
	sd	$17,0x20($29)
	lw	$3,0($29)
	lw	$2,0x164($3)
	sw	$0,0x33c($2)
	lw	$2,0xc90($20)
	nop	
loc_00175e18:
	addiu	$18,$0,0xa
	addiu	$17,$21,0x4ec0
	addiu	$19,$0,0x3c
	lui	$1,0xbf80
	mtc1	$1,$f12
	beqz	$2,loc_00175e40
	addiu	$5,$0,2
	lw	$4,0($29)
	jal	0x0014b270
	lw	$6,0xca0($20)
loc_00175e40:
	lw	$4,0x4ec0($21)
	lw	$3,4($17)
	mult	$2,$4,$18
	beql	$3,$0,loc_00175e54
	break	0,7
loc_00175e54:
	subu	$2,$19,$2
	div	$0,$2,$3
	mflo	$2
	slt	$2,$2,$16
	beqz	$2,loc_00175e8c
	mult	$2,$4,$18
	lw	$4,0($29)
	jal	0x0014af70
	lw	$5,-0x6e0c($28)
	bnel	$2,$0,loc_00175eb4
	sb	$23,0x58e0($20)
	lw	$3,4($17)
	lw	$4,0x4ec0($22)
	mult	$2,$4,$18
loc_00175e8c:
	beql	$3,$0,loc_00175e94
	break	0,7
loc_00175e94:
	subu	$2,$19,$2
	div	$0,$2,$3
	mflo	$2
	sll	$2,$2,1
	slt	$2,$2,$16
	beql	$2,$0,loc_00175eb8
	addiu	$16,$16,1
	sb	$23,0x58e0($20)
loc_00175eb4:
	addiu	$16,$16,1
loc_00175eb8:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00175e18
	lw	$2,0xc90($20)
	lui	$3,0x6b
	.set	macro
	.set	reorder
	.end	subGirlBrain_Busy
