	.text
	.p2align 3
	.globl	girlBrain_sub_175B6C
	.ent	girlBrain_sub_175B6C
girlBrain_sub_175B6C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$4,$4,-0x6b30
	sd	$16,0x10($29)
	jal	0x001a6e28
	lw	$16,0x164($2)
	lui	$2,0x17
	addiu	$3,$0,0x51
	addiu	$2,$2,0x5bd8
	sw	$3,0x30($16)
	sw	$2,0x14($16)
	sw	$0,0xd0($16)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_175B6C
