	.text
	.p2align 3
	.globl	girlBrain_sub_16D84C
	.ent	girlBrain_sub_16D84C
girlBrain_sub_16D84C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$4,$0,0x10
	lw	$3,0xc($2)
	bnel	$3,$4,loc_0016d868
	lw	$5,0x164($7)
	addiu	$9,$0,1
	.word	0x10000010
	sw	$9,0xe0($29)
loc_0016d868:
	addiu	$2,$0,3
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16D84C
