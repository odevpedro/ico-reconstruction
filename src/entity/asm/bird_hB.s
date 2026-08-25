	.text
	.p2align 3
	.globl	bird_hB
	.ent	bird_hB
bird_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	j	0x1e3fc8
	nop	
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	.set	macro
	.set	reorder
	.end	bird_hB
