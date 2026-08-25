	.text
	.p2align 3
	.globl	girlBrain_sub_175B4C
	.ent	girlBrain_sub_175B4C
girlBrain_sub_175B4C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jr	$31
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0x30
	sw	$4,0($29)
	sd	$31,0x20($29)
	lui	$4,0x56
	lw	$2,0($29)
	addiu	$4,$4,-0x6b30
	.set	macro
	.set	reorder
	.end	girlBrain_sub_175B4C
