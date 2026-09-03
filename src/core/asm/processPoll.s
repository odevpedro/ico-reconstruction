	.text
	.p2align 3
	.globl	processPoll
	.ent	processPoll
processPoll:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	sd	$31,0x10($29)
	bnez	$4,loc_0013d8c8
	sd	$16,0($29)
	lui	$16,0x6a
	jal	0x00100410
	addiu	$16,$16,0x6f30
	sll	$2,$2,2
	addu	$2,$2,$16
	lw	$4,0($2)
loc_0013d8c8:
	lw	$2,0x18($4)
	ld	$31,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	processPoll
