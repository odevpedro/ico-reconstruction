	.text
	.p2align 3
	.globl	attackch62_hA
	.ent	attackch62_hA
attackch62_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	lw	$3,-0x5c28($28)
	sd	$16,0($29)
	sd	$31,0x10($29)
	move	$16,$4
	lw	$2,0x15c($16)
	beqz	$3,loc_001bbf68
	lw	$2,0x800($2)
	lw	$2,4($2)
	bnez	$2,loc_001bbf6c
	ld	$31,0x10($29)
	jal	0x001118b8
	addiu	$4,$0,0xb
	jal	0x00112148
	addiu	$4,$0,1
	addiu	$4,$0,1
	addiu	$5,$0,5
	jal	0x00111fa8
	addiu	$6,$0,0x80
	jal	0x00105278
	nop	
	jal	0x00118678
	move	$4,$2
	jal	0x00105278
	nop	
	lw	$3,0x15c($16)
	addiu	$4,$2,0x30
	lw	$5,0xc($3)
	jal	0x00105f00
	addiu	$5,$5,0x30
	jal	0x001bbfd0
	move	$4,$16
	lui	$4,0x4c
	mov.s	$f12,$f0
	addiu	$4,$4,-0x1040
	addiu	$5,$0,4
	jal	0x0011e220
	addiu	$6,$0,4
	jal	0x0010f630
	nop	
loc_001bbf68:
	ld	$31,0x10($29)
loc_001bbf6c:
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	attackch62_hA
