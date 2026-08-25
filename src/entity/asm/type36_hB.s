	.text
	.p2align 3
	.globl	type36_hB
	.ent	type36_hB
type36_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$3,0x15c($4)
	lw	$4,0x800($3)
	lw	$5,0x74($3)
	lw	$2,0x5c($4)
	jr	$31
	sw	$5,0x16c($2)
	.set	macro
	.set	reorder
	.end	type36_hB
