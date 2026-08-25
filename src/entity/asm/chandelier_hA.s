	.text
	.p2align 3
	.globl	chandelier_hA
	.ent	chandelier_hA
chandelier_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	sd	$31,0x10($29)
	sd	$16,0($29)
	lw	$16,0x15c($4)
	lw	$2,0x74($16)
	beqz	$2,loc_001c34a8
	ld	$31,0x10($29)
	jal	0x0010ecd8
	nop	
	move	$4,$16
	ld	$31,0x10($29)
	ld	$16,0($29)
	j	0x10eca0
	addiu	$29,$29,0x20
loc_001c34a8:
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	chandelier_hA
