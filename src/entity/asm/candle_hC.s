	.text
	.p2align 3
	.globl	candle_hC
	.ent	candle_hC
candle_hC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x60
	sd	$16,0($29)
	sd	$31,0x50($29)
	move	$16,$5
	sd	$20,0x40($29)
	sd	$19,0x30($29)
	sd	$18,0x20($29)
	sd	$17,0x10($29)
	lw	$17,0x15c($4)
	lw	$5,8($17)
	slti	$2,$5,2
	bnez	$2,loc_001c30c4
	lui	$6,0x62
	lw	$4,-0x68e0($28)
	sll	$5,$5,3
	addiu	$6,$6,-0x77a8
	addiu	$7,$0,0x18
	jal	0x0013a0f8
	move	$16,$0
	lw	$3,8($17)
	.word	0x1860002f
	move	$18,$2
	lui	$19,0x27
	lui	$20,0x62
loc_001c3050:
	jal	0x00105278
	nop	
	lw	$3,0xc($17)
	sll	$5,$16,6
	move	$4,$2
	jal	0x00105f20
	addu	$5,$3,$5
	mtc1	$0,$f12
	lui	$1,0xc220
	mtc1	$1,$f13
	jal	0x00105308
	mov.s	$f14,$f12
	jal	0x00105278
	nop	
	addiu	$5,$2,0x30
	addiu	$4,$0,4
	jal	0x001e8b48
	addiu	$6,$19,0x6140
	sll	$3,$16,3
	addu	$3,$3,$18
	addiu	$16,$16,1
	sw	$2,0($3)
	sw	$0,4($3)
	lw	$2,8($17)
	slt	$2,$16,$2
	bnez	$2,loc_001c3050
	nop	
	.word	0x10000011
	nop	
loc_001c30c4:
	lw	$4,-0x68e0($28)
	addiu	$6,$6,-0x77a8
	addiu	$5,$0,8
	addiu	$7,$0,0x23
	jal	0x0013a0f8
	lui	$20,0x62
	lui	$6,0x27
	move	$18,$2
	move	$5,$16
	addiu	$6,$6,0x6140
	jal	0x001e8b48
	.set	macro
	.set	reorder
	.end	candle_hC
