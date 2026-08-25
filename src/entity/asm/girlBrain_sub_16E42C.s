	.text
	.p2align 3
	.globl	girlBrain_sub_16E42C
	.ent	girlBrain_sub_16E42C
girlBrain_sub_16E42C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	lui	$1,0x3f80
	mtc1	$1,$f20
	b	loc_0016e444
	nop	
	mtc1	$0,$f20
loc_0016e444:
	lui	$1,0x3f00
	mtc1	$1,$f21
	lw	$4,0($29)
	.word	0x4614a834
	nop	
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E42C
