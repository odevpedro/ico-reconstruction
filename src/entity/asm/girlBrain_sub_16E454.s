	.text
	.p2align 3
	.globl	girlBrain_sub_16E454
	.ent	girlBrain_sub_16E454
girlBrain_sub_16E454:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	bc1f	loc_0016e478
	lw	$16,0x164($4)
	jal	0x00201d50
	nop	
	.word	0x5040004c
	swc1	$f20,0x33c($16)
	.word	0x1000004a
	swc1	$f21,0x33c($16)
loc_0016e478:
	.word	0x10000048
	swc1	$f20,0x33c($16)
	lwc1	$f1,0x3e8($16)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E454
