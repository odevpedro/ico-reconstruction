	.text
	.p2align 3
	.globl	gif_test
	.ent	gif_test
gif_test:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
loc_00179748:
	lui	$9,0x7fff
	addu	$2,$18,$8
	ori	$9,$9,0xffff
	sb	$14,0($2)
	move	$7,$0
	move	$10,$19
	move	$6,$17
	move	$11,$0
loc_00179768:
	addu	$2,$18,$7
	lb	$3,0($2)
	bnel	$3,$0,loc_001797d4
	addiu	$7,$7,1
	sll	$4,$8,2
	addu	$2,$4,$16
	lw	$3,0($2)
	addu	$3,$11,$3
	lw	$5,0($3)
	slt	$2,$12,$5
	bnez	$2,loc_001797bc
	lw	$3,0($6)
	addu	$4,$4,$17
	lw	$2,0($4)
	addu	$2,$2,$5
	slt	$3,$2,$3
	beql	$3,$0,loc_001797bc
	lw	$3,0($6)
	sw	$2,0($6)
	sw	$8,0($10)
	lw	$3,0($6)
loc_001797bc:
	slt	$2,$3,$9
	beql	$2,$0,loc_001797d4
	addiu	$7,$7,1
	move	$9,$3
	move	$13,$7
	addiu	$7,$7,1
loc_001797d4:
	addiu	$10,$10,4
	addiu	$6,$6,4
	slti	$2,$7,0x5e
	bnez	$2,loc_00179768
	addiu	$11,$11,4
	slt	$2,$12,$9
	beqz	$2,loc_00179748
	move	$8,$13
	addu	$2,$22,$17
	lui	$3,0x7fff
	lw	$4,0($2)
	ori	$3,$3,0xfffe
	slt	$3,$3,$4
	bnez	$3,loc_00179844
	addiu	$8,$0,-1
	addu	$3,$22,$19
	lw	$2,0($3)
	beq	$2,$21,loc_00179844
	move	$8,$30
	move	$4,$22
	addu	$2,$4,$19
loc_00179828:
	lw	$8,0($2)
	sll	$3,$8,2
	move	$4,$3
	addu	$2,$4,$19
	lw	$3,0($2)
	bne	$3,$21,loc_00179828
	nop	
loc_00179844:
	ld	$31,0x90($29)
	move	$2,$8
	ld	$30,0x80($29)
	ld	$23,0x70($29)
	ld	$22,0x60($29)
	ld	$21,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	.set	macro
	.set	reorder
	.end	gif_test
