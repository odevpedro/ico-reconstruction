	.text
	.p2align 3
	.globl	cagefix_hB
	.ent	cagefix_hB
cagefix_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	sd	$17,0x10($29)
	move	$17,$4
	sd	$16,0($29)
	sd	$31,0x20($29)
	jal	0x0013eb50
	addiu	$4,$0,0x2b
	move	$16,$2
	beqz	$16,loc_001c2f8c
	ld	$31,0x20($29)
	jal	0x00105278
	nop	
	lw	$3,0x15c($17)
	move	$4,$2
	jal	0x00105f20
	lw	$5,0xc($3)
	jal	0x00105278
	nop	
	lw	$3,0x15c($17)
	move	$4,$16
	ld	$31,0x20($29)
	addiu	$5,$2,0x30
	lw	$6,0x10($3)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x1c22f0
	addiu	$29,$29,0x30
loc_001c2f8c:
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	.set	macro
	.set	reorder
	.end	cagefix_hB
