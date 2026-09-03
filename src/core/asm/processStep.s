	.text
	.p2align 3
	.globl	processStep
	.ent	processStep
processStep:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x10
	sd	$31,0($29)
	jal	0x00100450
	lw	$4,0x30($4)
	ld	$31,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	processStep
