	.text
	.p2align 3
	.globl	candle_hB
	.ent	candle_hB
candle_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x80
	sd	$19,0x30($29)
	sd	$31,0x60($29)
	move	$19,$4
	sd	$21,0x50($29)
	sd	$20,0x40($29)
	sd	$17,0x10($29)
	sd	$16,0($29)
	swc1	$f20,0x70($29)
	sd	$18,0x20($29)
	lw	$18,0x15c($19)
	lw	$2,0x74($18)
	beqz	$2,loc_001c3244
	lw	$4,0x800($18)
	lw	$3,8($18)
	slti	$2,$3,2
	bnez	$2,loc_001c3248
	ld	$31,0x60($29)
	blez	$3,loc_001c3248
	move	$17,$0
	mtc1	$0,$f20
	move	$16,$4
	addiu	$21,$0,-1
	lui	$20,0x27
loc_001c31d8:
	jal	0x00105278
	nop	
	lw	$3,0x15c($19)
	sll	$6,$17,6
	move	$4,$2
	lw	$5,0xc($3)
	jal	0x00105f20
	addu	$5,$5,$6
	lui	$1,0xc220
	mtc1	$1,$f13
	mov.s	$f12,$f20
	jal	0x00105308
	mov.s	$f14,$f20
	lw	$2,0($16)
	beql	$2,$21,loc_001c3234
	lw	$2,8($18)
	jal	0x00105278
	nop	
	lw	$4,0($16)
	addiu	$5,$2,0x30
	jal	0x001e8598
	addiu	$6,$20,0x6140
	lw	$2,8($18)
loc_001c3234:
	addiu	$17,$17,1
	slt	$2,$17,$2
	bnez	$2,loc_001c31d8
	addiu	$16,$16,8
loc_001c3244:
	ld	$31,0x60($29)
loc_001c3248:
	ld	$21,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	lwc1	$f20,0x70($29)
	jr	$31
	addiu	$29,$29,0x80
	.set	macro
	.set	reorder
	.end	candle_hB
