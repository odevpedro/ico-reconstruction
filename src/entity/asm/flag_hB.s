	.text
	.p2align 3
	.globl	flag_hB
	.ent	flag_hB
flag_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x40
	sd	$18,0x20($29)
	sd	$31,0x30($29)
	move	$18,$4
	sd	$17,0x10($29)
	sd	$16,0($29)
	lw	$2,0x15c($18)
	lw	$17,0x800($2)
	lw	$3,0($2)
	beqz	$3,loc_001d0130
	lw	$16,4($17)
	lw	$2,0x16c($3)
	beqz	$2,loc_001d01d4
	ld	$31,0x30($29)
loc_001d0130:
	jal	0x00105278
	nop	
	move	$4,$2
	jal	0x00104140
	move	$5,$18
	lw	$3,0($17)
	bltz	$3,loc_001d01d0
	slti	$2,$3,2
	bnez	$2,loc_001d0168
	addiu	$2,$0,2
	beq	$3,$2,loc_001d01a0
	ld	$31,0x30($29)
	b	loc_001d01d8
	ld	$18,0x20($29)
loc_001d0168:
	jal	0x00105278
	lw	$16,4($16)
	lw	$9,0xc($17)
	move	$10,$0
	lw	$5,8($16)
	move	$7,$2
	lw	$8,8($17)
	move	$6,$0
	lw	$4,4($16)
	movn	$10,$18,$9
	jal	0x001c4a28
	move	$11,$0
	b	loc_001d01d4
	ld	$31,0x30($29)
loc_001d01a0:
	jal	0x00105278
	lw	$16,4($16)
	lw	$9,0xc($17)
	move	$10,$0
	lw	$5,8($16)
	move	$7,$2
	lw	$8,8($17)
	move	$6,$0
	lw	$4,4($16)
	movn	$10,$18,$9
	jal	0x001c4a28
	addiu	$11,$0,1
loc_001d01d0:
	ld	$31,0x30($29)
loc_001d01d4:
	ld	$18,0x20($29)
loc_001d01d8:
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x40
	.set	macro
	.set	reorder
	.end	flag_hB
