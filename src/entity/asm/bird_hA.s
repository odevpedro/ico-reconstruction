	.text
	.p2align 3
	.globl	bird_hA
	.ent	bird_hA
bird_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	sd	$31,0x10($29)
	jal	0x0010ecc0
	move	$16,$4
	lw	$2,0x15c($16)
	lw	$16,0x800($2)
	lw	$3,0x30($16)
	beqz	$3,loc_001970bc
	lw	$3,-0x6f60($28)
	jal	0x0012a7f8
	addiu	$4,$16,0x30
	bnel	$2,$0,loc_001970b8
	sw	$0,0x30($16)
loc_001970b8:
	lw	$3,-0x6f60($28)
loc_001970bc:
	addiu	$2,$0,0x52
	bne	$3,$2,loc_001970d4
	ld	$31,0x10($29)
	ld	$16,0($29)
	j	0x1998c0
	addiu	$29,$29,0x20
loc_001970d4:
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	bird_hA
