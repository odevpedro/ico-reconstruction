	.text
	.p2align 3
	.globl	girlBrain_sub_16E538
	.ent	girlBrain_sub_16E538
girlBrain_sub_16E538:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x45000007
	lw	$16,0x164($4)
	jal	0x00201d50
	nop	
	.word	0x50400004
	swc1	$f20,0x33c($16)
	.word	0x10000002
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E538
