	.text
	.p2align 3
	.globl	girlBrain_sub_16DF10
	.ent	girlBrain_sub_16DF10
girlBrain_sub_16DF10:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	lw	$2,0x148($5)
	.word	0x14620006
	addiu	$3,$0,0x4e
	lw	$4,0($29)
	jal	0x0016ac10
	move	$5,$21
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16DF10
