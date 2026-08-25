	.text
	.p2align 3
	.globl	girlBrain_sub_16F5F0
	.ent	girlBrain_sub_16F5F0
girlBrain_sub_16F5F0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x46001034
	nop	
	bc1f	loc_0016f610
	move	$5,$16
	jal	0x00194398
	move	$4,$21
	lwc1	$f1,-0x7b40($28)
	.word	0x46010034
loc_0016f610:
	addiu	$18,$18,1
	slt	$2,$18,$23
	.word	0x5440ffd9
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16F5F0
