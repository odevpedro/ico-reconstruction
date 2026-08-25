	.text
	.p2align 3
	.globl	girl_hA
	.ent	girl_hA
girl_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	sd	$31,0x10($29)
	jal	0x0010ecd8
	move	$16,$4
	jal	0x0010ecb8
	move	$4,$16
	jal	0x001d1580
	move	$4,$16
	move	$4,$16
	ld	$31,0x10($29)
	ld	$16,0($29)
	j	0x1d19c0
	addiu	$29,$29,0x20
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	girl_hA
