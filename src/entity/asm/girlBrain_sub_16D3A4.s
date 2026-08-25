	.text
	.p2align 3
	.globl	girlBrain_sub_16D3A4
	.ent	girlBrain_sub_16D3A4
girlBrain_sub_16D3A4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bnez	$2,loc_0016d464
	move	$2,$16
	addiu	$16,$29,0x20
	move	$5,$20
	move	$4,$16
	jal	0x00145900
	addiu	$6,$0,0x23
	move	$4,$16
	jal	0x00194508
	move	$5,$19
	move	$3,$2
	slti	$2,$18,0x5a
	beqz	$2,loc_0016d3e4
	mult	$2,$17,$3
	bltz	$2,loc_0016d3e8
	move	$2,$17
loc_0016d3e4:
	move	$2,$3
loc_0016d3e8:
	blez	$2,loc_0016d424
	lui	$4,0x6b
	addiu	$5,$0,2
	lw	$2,-0x4220($4)
	slt	$2,$5,$2
	bnez	$2,loc_0016d45c
	addiu	$3,$4,-0x4220
	lwc1	$f0,0x10($29)
	lwc1	$f2,0x14($29)
	lwc1	$f1,0x18($29)
	sw	$5,-0x4220($4)
	swc1	$f1,0x18($3)
	sw	$5,4($3)
	b	loc_0016d458
	swc1	$f0,0x10($3)
loc_0016d424:
	addiu	$5,$0,2
	lw	$2,-0x4220($4)
	slt	$2,$5,$2
	bnez	$2,loc_0016d45c
	addiu	$3,$4,-0x4220
	lwc1	$f1,0x10($29)
	addiu	$2,$0,1
	lwc1	$f2,0x14($29)
	lwc1	$f0,0x18($29)
	sw	$5,-0x4220($4)
	swc1	$f0,0x18($3)
	sw	$2,4($3)
	swc1	$f1,0x10($3)
loc_0016d458:
	swc1	$f2,0x14($3)
loc_0016d45c:
	addiu	$16,$0,1
	move	$2,$16
loc_0016d464:
	ld	$31,0x80($29)
	ld	$20,0x70($29)
	ld	$19,0x60($29)
	ld	$18,0x50($29)
	ld	$17,0x40($29)
	ld	$16,0x30($29)
	jr	$31
	addiu	$29,$29,0x90
	nop	
	addiu	$29,$29,-0x80
	lw	$5,-0x6e0c($28)
	sd	$18,0x50($29)
	move	$18,$4
	sd	$19,0x60($29)
	sd	$17,0x40($29)
	addiu	$4,$29,0x10
	sd	$16,0x30($29)
	addiu	$17,$0,-1
	sd	$31,0x70($29)
	jal	0x0010a498
	addiu	$16,$0,-1
	move	$19,$0
	lw	$5,-0x6e08($28)
	jal	0x0010a498
	addiu	$4,$29,0x20
	lwc1	$f1,0x24($29)
	lui	$1,0x44c8
	mtc1	$1,$f0
	.word	0x46010034
	nop	
	bc1fl	loc_0016d534
	lwc1	$f1,0x14($29)
	lwc1	$f0,-0x7b7c($28)
	.word	0x46000834
	nop	
	bc1f	loc_0016d528
	lwc1	$f1,0x28($29)
	lui	$1,0xc316
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0016d528
	move	$17,$0
	lwc1	$f0,-0x7b78($28)
	.word	0x46010034
	nop	
	bc1f	loc_0016d528
	addiu	$17,$0,1
	addiu	$17,$0,2
loc_0016d528:
	lwc1	$f1,0x14($29)
	lui	$1,0x44c8
	mtc1	$1,$f0
loc_0016d534:
	.word	0x46010034
	nop	
	bc1f	loc_0016d58c
	addiu	$2,$0,-1
	lwc1	$f0,-0x7b74($28)
	.word	0x46000834
	nop	
	bc1f	loc_0016d58c
	lwc1	$f1,0x18($29)
	lui	$1,0xc316
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0016d58c
	move	$16,$0
	lwc1	$f0,-0x7b70($28)
	.word	0x46010034
	nop	
	bc1f	loc_0016d58c
	addiu	$16,$0,1
	addiu	$16,$0,2
	addiu	$2,$0,-1
loc_0016d58c:
	.word	0x52020057
	move	$2,$0
	bne	$17,$2,loc_0016d5a4
	addiu	$2,$0,1
	.word	0x10000053
	move	$2,$0
loc_0016d5a4:
	.word	0x12220052
	ld	$31,0x70($29)
	.word	0x1602001b
	lw	$2,-0x6e0c($28)
	lw	$3,0x15c($2)
	lw	$4,0($3)
	.word	0x10800015
	addiu	$2,$0,0x11
	lw	$3,0xc($4)
	.word	0x14620049
	addiu	$2,$0,1
	jal	0x0015eff8
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16D3A4
