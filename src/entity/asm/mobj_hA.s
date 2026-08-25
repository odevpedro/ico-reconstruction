	.text
	.p2align 3
	.globl	mobj_hA
	.ent	mobj_hA
mobj_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	j	0x10eca8
	nop	
	j	0x10ec98
	lw	$4,0x15c($4)
	j	0x10ecb8
	nop	
	lui	$2,0x27
	lw	$4,-0x5164($28)
	j	0x244980
	lw	$5,0x6214($2)
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	mobj_hA
