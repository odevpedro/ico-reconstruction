	.text
	.p2align 3
	.globl	boyAI_sub_14E314
	.ent	boyAI_sub_14E314
boyAI_sub_14E314:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$18,$4
	sd	$31,0x80($29)
	addiu	$4,$0,2
	sd	$23,0x70($29)
	move	$17,$0
	sd	$22,0x60($29)
	sd	$21,0x50($29)
	sd	$19,0x30($29)
	sd	$16,0($29)
	jal	0x0014b358
	lw	$16,0x15c($18)
	lwc1	$f1,0x560($16)
	.word	0x46010034
	nop	
	.word	0x45010044
	ld	$31,0x80($29)
	lw	$2,0x164($18)
	addiu	$4,$0,0x16
	lw	$3,0x30($2)
	.word	0x1064003f
	lw	$2,-0x6e00($28)
	.word	0x1440003e
	ld	$23,0x70($29)
	lui	$2,0x6b
	addiu	$16,$0,1
	move	$23,$2
	addiu	$19,$2,-0x5520
	addiu	$22,$0,1
	ori	$21,$0,0x8000
	dsll	$21,$21,0x14
	move	$4,$18
	jal	0x0010a048
	move	$5,$16
	.word	0x10400028
	addiu	$3,$23,-0x5520
	lw	$2,0x10($3)
	bgez	$2,loc_0014e3b4
	move	$20,$0
	.word	0x1000002b
	sw	$16,0x10($3)
loc_0014e3b4:
	.word	0x50500022
	addiu	$16,$16,1
	jal	0x0014a0d8
	nop	
	beqz	$2,loc_0014e3d4
	lw	$2,-0x6e08($28)
	.word	0x10000006
	addiu	$17,$0,1
loc_0014e3d4:
	.word	0x10400004
	lw	$4,-0x6f60($28)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14E314
