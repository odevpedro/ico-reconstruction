	.text
	.p2align 3
	.globl	boyAI_sub_153928
	.ent	boyAI_sub_153928
boyAI_sub_153928:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$4,0x56
	lw	$5,0($29)
	addiu	$4,$4,0x5060
	lw	$3,0x15c($5)
	lw	$2,0x4a0($3)
	mult	$2,$2,$6
	.set	macro
	.set	reorder
	.end	boyAI_sub_153928
