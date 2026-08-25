	.text
	.p2align 3
	.globl	girlBrain_sub_170578
	.ent	girlBrain_sub_170578
girlBrain_sub_170578:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lwc1	$f1,0x54($29)
	lwc1	$f2,0x58($29)
	swc1	$f0,0xf0($29)
	swc1	$f1,0xf4($29)
	swc1	$f2,0xf8($29)
	lwc1	$f3,0xf4($29)
	addiu	$4,$29,0x30
	lwc1	$f0,0xf0($29)
	add.s	$f1,$f3,$f22
	lwc1	$f2,0xf8($29)
	swc1	$f0,0x40($29)
	swc1	$f2,0x48($29)
	swc1	$f1,0x44($29)
	swc1	$f0,0x30($29)
	swc1	$f3,0x34($29)
	jal	0x00168538
	swc1	$f2,0x38($29)
	lw	$2,0xc4($29)
	bnez	$2,loc_001705d0
	lwc1	$f0,0x54($29)
	.word	0x10000005
	move	$22,$0
loc_001705d0:
	lui	$1,0x4120
	.set	macro
	.set	reorder
	.end	girlBrain_sub_170578
