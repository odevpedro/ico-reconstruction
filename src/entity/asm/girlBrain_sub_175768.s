	.text
	.p2align 3
	.globl	girlBrain_sub_175768
	.ent	girlBrain_sub_175768
girlBrain_sub_175768:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bne	$2,$5,loc_00175788
	nop	
	lw	$2,0($29)
	lw	$3,0x164($2)
	lw	$4,0x678($3)
	lw	$2,0x830($4)
	xori	$2,$2,4
	sltiu	$16,$2,1
loc_00175788:
	beqz	$16,loc_0017579c
	addiu	$5,$0,0xb7
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
loc_0017579c:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x13a
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00175788
	nop	
	addiu	$29,$29,-0x30
	sw	$4,0($29)
	sd	$31,0x20($29)
	lui	$4,0x56
	lw	$2,0($29)
	addiu	$4,$4,-0x6500
	sd	$16,0x10($29)
	jal	0x001a6e28
	lw	$16,0x164($2)
	addiu	$2,$0,0xf
	move	$4,$0
	jal	0x00203aa0
	sw	$2,0x30($16)
	ld	$31,0x20($29)
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x30
	nop	
	lui	$7,0x27
	addiu	$6,$0,0xa
	lw	$2,0x4ec0($7)
	addiu	$29,$29,-0x70
	sd	$16,0x10($29)
	addiu	$8,$7,0x4ec0
	mult	$2,$2,$6
	sd	$31,0x60($29)
	sd	$20,0x50($29)
	addiu	$3,$0,0x3c
	sd	$19,0x40($29)
	sd	$18,0x30($29)
	sd	$17,0x20($29)
	subu	$3,$3,$2
	lw	$5,4($8)
	sw	$4,0($29)
	div	$0,$3,$5
	beql	$5,$0,loc_0017584c
	break	0,7
loc_0017584c:
	mflo	$2
	sll	$2,$2,1
	.word	0x1840001b
	move	$16,$0
	move	$20,$7
	move	$19,$8
	addiu	$18,$0,0xa
	addiu	$17,$0,0x3c
	.set	macro
	.set	reorder
	.end	girlBrain_sub_175768
