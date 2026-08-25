	.text
	.p2align 3
	.globl	type5_hA
	.ent	type5_hA
type5_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	sd	$17,0x10($29)
	sd	$31,0x20($29)
	move	$17,$4
	sd	$16,0($29)
	lw	$4,0x15c($17)
	lw	$16,0x800($4)
	lw	$2,0x38($16)
	beqz	$2,loc_001ce654
	nop	
	jal	0x001224e0
	nop	
	lw	$3,0x15c($17)
	mtc1	$0,$f1
	lw	$2,0x840($3)
	lwc1	$f0,0x30($2)
	.word	0x46010032
	bc1f	loc_001ce654
	nop	
	jal	0x001cf998
	lw	$4,0x18($16)
	jal	0x001cf998
	lw	$4,0x20($16)
loc_001ce654:
	jal	0x001cf770
	lw	$4,0x28($16)
	lw	$3,0x10($16)
	beqz	$3,loc_001ce67c
	ld	$31,0x20($29)
	move	$4,$17
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x1cdb28
	addiu	$29,$29,0x30
loc_001ce67c:
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	.set	macro
	.set	reorder
	.end	type5_hA
