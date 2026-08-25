	.text
	.p2align 3
	.globl	girlBrain_sub_16CF94
	.ent	girlBrain_sub_16CF94
girlBrain_sub_16CF94:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$7,$29,0x60
	sub.s	$f1,$f1,$f2
	lwc1	$f3,0x40($29)
	lwc1	$f2,0x48($29)
	lw	$6,-0x6e0c($28)
	add.s	$f0,$f1,$f0
	swc1	$f3,0x50($29)
	mtc1	$0,$f12
	swc1	$f2,0x58($29)
	swc1	$f0,0x54($29)
	jal	0x0014aa28
	swc1	$f1,0x44($29)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16CF94
