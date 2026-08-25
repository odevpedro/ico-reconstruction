	.text
	.p2align 3
	.globl	boyAI_sub_150498
	.ent	boyAI_sub_150498
boyAI_sub_150498:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$4,$16
	jal	0x00194020
	move	$5,$16
	jal	0x0018cec0
	nop	
	move	$4,$2
	move	$5,$16
	addiu	$6,$0,5
	jal	0x0018cd98
	addiu	$7,$0,1
	ld	$31,0x20($29)
	ld	$16,0x10($29)
	lwc1	$f20,0x30($29)
	jr	$31
	addiu	$29,$29,0x40
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_150498
