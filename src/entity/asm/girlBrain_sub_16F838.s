	.text
	.p2align 3
	.globl	girlBrain_sub_16F838
	.ent	girlBrain_sub_16F838
girlBrain_sub_16F838:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	.word	0x45010044
	addiu	$17,$17,1
	lw	$2,-0x20($19)
	slt	$2,$17,$2
	.word	0x1440ffee
	addiu	$16,$0,0x30
	addiu	$2,$0,1
	andi	$2,$2,0xff
	.word	0x1040003e
	lui	$4,0x6b
	lw	$5,0xd8($29)
	addiu	$16,$4,-0x44c0
	move	$6,$30
	addu	$16,$22,$16
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16F838
