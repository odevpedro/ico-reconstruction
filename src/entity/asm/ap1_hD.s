	.text
	.p2align 3
	.globl	ap1_hD
	.ent	ap1_hD
ap1_hD:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0xa0
	sd	$16,0x20($29)
	move	$16,$4
	sd	$21,0x70($29)
	sd	$19,0x50($29)
	move	$21,$0
	sd	$18,0x40($29)
	sd	$31,0x90($29)
	addiu	$18,$16,0x54
	sd	$22,0x80($29)
	sd	$20,0x60($29)
	sd	$17,0x30($29)
	lw	$4,4($18)
	blez	$4,loc_001bb680
	addiu	$19,$16,0x5c
	addiu	$22,$0,-2
	dsll	$22,$22,0x10
	ori	$22,$22,0xffff
	dsll	$22,$22,0x10
	ori	$22,$22,0xffff
	addiu	$20,$0,1
	nop	
loc_001bb438:
	lw	$3,0($19)
	addiu	$2,$0,0x20
	beql	$3,$2,loc_001bb4c0
	lw	$4,0x164($16)
	sltiu	$2,$3,0x21
	beqz	$2,loc_001bb488
	addiu	$2,$0,0x1a
	beq	$3,$2,loc_001bb5cc
	sltiu	$2,$3,0x1b
	beqz	$2,loc_001bb474
	addiu	$2,$0,0xd
	beql	$3,$2,loc_001bb548
	move	$4,$16
	b	loc_001bb674
	addiu	$21,$21,1
loc_001bb474:
	addiu	$2,$0,0x1f
	beql	$3,$2,loc_001bb4d0
	lw	$4,0x164($16)
	b	loc_001bb674
	addiu	$21,$21,1
loc_001bb488:
	addiu	$2,$0,0x26
	beq	$3,$2,loc_001bb5cc
	sltiu	$2,$3,0x27
	beqz	$2,loc_001bb4ac
	addiu	$2,$0,0x22
	beql	$3,$2,loc_001bb4e8
	lw	$6,0x164($16)
	b	loc_001bb674
	addiu	$21,$21,1
loc_001bb4ac:
	addiu	$2,$0,0xcf
	beql	$3,$2,loc_001bb63c
	lw	$3,0x164($16)
	b	loc_001bb674
	addiu	$21,$21,1
loc_001bb4c0:
	addiu	$3,$0,0x400
	ld	$2,0x20($4)
	b	loc_001bb4dc
	or	$2,$2,$3
loc_001bb4d0:
	addiu	$3,$0,-0x401
	ld	$2,0x20($4)
	and	$2,$2,$3
loc_001bb4dc:
	sd	$2,0x20($4)
	b	loc_001bb670
	lw	$4,4($18)
loc_001bb4e8:
	addiu	$5,$0,4
	lw	$2,0x30($6)
	beql	$2,$5,loc_001bb674
	addiu	$21,$21,1
	ld	$3,0x18($6)
	move	$4,$29
	sw	$5,0x30($6)
	and	$3,$3,$22
	move	$5,$16
	sd	$3,0x18($6)
	lw	$2,0x164($16)
	jal	0x00104508
	sb	$20,0x1ca($2)
	addiu	$4,$29,0x10
	jal	0x00102850
	move	$5,$16
	addiu	$4,$0,0x31
	move	$5,$29
	jal	0x001e8b48
	addiu	$6,$29,0x10
	jal	0x001bb390
	move	$4,$16
	b	loc_001bb670
	lw	$4,4($18)
loc_001bb548:
	jal	0x001bb390
	addiu	$17,$0,4
	lw	$3,0x164($16)
	lw	$2,0x30($3)
	beq	$2,$17,loc_001bb58c
	move	$4,$29
	jal	0x00104508
	move	$5,$16
	addiu	$4,$29,0x10
	jal	0x00102850
	move	$5,$16
	addiu	$4,$0,0xc
	move	$5,$29
	jal	0x001e8b48
	addiu	$6,$29,0x10
	lw	$3,0x164($16)
	lw	$2,0x30($3)
loc_001bb58c:
	beql	$2,$17,loc_001bb5b0
	lw	$4,-0x6e04($28)
	ld	$2,0x18($3)
	sw	$17,0x30($3)
	and	$2,$2,$22
	sd	$2,0x18($3)
	lw	$3,0x164($16)
	sb	$20,0x1ca($3)
	lw	$4,-0x6e04($28)
loc_001bb5b0:
	jal	0x0013b610
	addiu	$5,$0,0x11
	move	$4,$16
	jal	0x001d12a8
	addiu	$5,$0,0x64
	b	loc_001bb670
	lw	$4,4($18)
loc_001bb5cc:
	lw	$3,0x164($16)
	addiu	$17,$0,4
	lw	$2,0x30($3)
	beq	$2,$17,loc_001bb60c
	move	$4,$29
	jal	0x00104508
	move	$5,$16
	addiu	$4,$29,0x10
	jal	0x00102850
	move	$5,$16
	addiu	$4,$0,0xc
	move	$5,$29
	jal	0x001e8b48
	addiu	$6,$29,0x10
	lw	$3,0x164($16)
	lw	$2,0x30($3)
loc_001bb60c:
	beq	$2,$17,loc_001bb62c
	nop	
	ld	$2,0x18($3)
	sw	$17,0x30($3)
	and	$2,$2,$22
	sd	$2,0x18($3)
	lw	$3,0x164($16)
	sb	$20,0x1ca($3)
loc_001bb62c:
	jal	0x001bb390
	move	$4,$16
	b	loc_001bb670
	lw	$4,4($18)
loc_001bb63c:
	addiu	$4,$0,4
	lw	$2,0x30($3)
	beq	$2,$4,loc_001bb664
	nop	
	ld	$2,0x18($3)
	sw	$4,0x30($3)
	and	$2,$2,$22
	sd	$2,0x18($3)
	lw	$3,0x164($16)
	sb	$20,0x1ca($3)
loc_001bb664:
	jal	0x001bb390
	move	$4,$16
	lw	$4,4($18)
loc_001bb670:
	addiu	$21,$21,1
loc_001bb674:
	slt	$2,$21,$4
	bnez	$2,loc_001bb438
	addiu	$19,$19,8
loc_001bb680:
	sw	$0,4($18)
	ld	$31,0x90($29)
	ld	$22,0x80($29)
	ld	$21,0x70($29)
	ld	$20,0x60($29)
	ld	$19,0x50($29)
	ld	$18,0x40($29)
	ld	$17,0x30($29)
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0xa0
	.set	macro
	.set	reorder
	.end	ap1_hD
