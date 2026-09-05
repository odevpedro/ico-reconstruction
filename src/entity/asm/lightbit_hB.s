	.text
	.globl	lightbit_hB
	.ent	lightbit_hB
lightbit_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x15c($4)
	lw	$3,0x800($2)
	j	0x1f0098
	lw	$4,0x20($3)
	lw	$2,0x15c($4)
	lw	$5,0x800($2)
	lw	$4,0x20($5)
	j	0x1f0260
	addiu	$5,$5,0x10
	.set	macro
	.set	reorder
	.end	lightbit_hB
