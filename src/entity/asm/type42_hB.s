	.text
	.p2align 3
	.globl	type42_hB
	.ent	type42_hB
type42_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	move	$16,$4
	sd	$31,0x10($29)
	jal	0x0013eb50
	addiu	$4,$0,0x15
	beqz	$2,loc_001e98bc
	ld	$31,0x10($29)
	move	$5,$16
	ld	$16,0($29)
	move	$4,$2
	j	0x190430
	addiu	$29,$29,0x20
loc_001e98bc:
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	type42_hB
