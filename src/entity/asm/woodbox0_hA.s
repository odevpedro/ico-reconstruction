	.text
	.p2align 3
	.globl	woodbox0_hA
	.ent	woodbox0_hA
woodbox0_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x30
	sd	$17,0x10($29)
	sd	$31,0x20($29)
	move	$17,$4
	sd	$16,0($29)
	lw	$2,0x15c($17)
	jal	0x0010ecd8
	lw	$16,0x800($2)
	jal	0x0010ecb8
	move	$4,$17
	lw	$2,0x58($16)
	beqz	$2,loc_001c0610
	lui	$2,0x27
	jal	0x001bd668
	move	$4,$17
	lui	$2,0x27
loc_001c0610:
	lw	$3,0x4ed4($2)
	.word	0x10600006
	move	$4,$17
	jal	0x001d12c0
	addiu	$5,$0,1
	lw	$2,0x15c($17)
	lw	$3,0x800($2)
	sw	$0,0x138($3)
	.set	macro
	.set	reorder
	.end	woodbox0_hA
