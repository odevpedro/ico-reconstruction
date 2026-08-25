	.text
	.p2align 3
	.globl	darkvolume_hA
	.ent	darkvolume_hA
darkvolume_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	darkvolume_hA
