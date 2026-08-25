	.text
	.p2align 3
	.globl	type20_hB
	.ent	type20_hB
type20_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x40
	sd	$16,0x10($29)
	sd	$17,0x20($29)
	sd	$31,0x30($29)
	jal	0x001e9588
	move	$16,$4
	lw	$17,-0x6e0c($28)
	move	$4,$29
	lw	$2,0x15c($16)
	move	$5,$17
	jal	0x00104508
	lw	$16,0x800($2)
	lw	$4,0($16)
	lui	$1,0x4348
	mtc1	$1,$f12
	jal	0x001c8d08
	move	$5,$29
	mov.s	$f1,$f0
	mtc1	$0,$f0
	.word	0x46010034
	nop	
	bc1f	loc_001e9878
	ld	$31,0x30($29)
	lw	$2,0x15c($17)
	swc1	$f1,0x618($2)
	nop	
loc_001e9878:
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x40
	.set	macro
	.set	reorder
	.end	type20_hB
