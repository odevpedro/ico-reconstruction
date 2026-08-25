	.text
	.p2align 3
	.globl	boyAI_sub_1518CC
	.ent	boyAI_sub_1518CC
boyAI_sub_1518CC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x0014a100
	addiu	$6,$0,0x12
	move	$4,$17
	jal	0x001943c8
	move	$5,$18
	lw	$2,0x4ec0($19)
	addiu	$3,$0,0xa
	.set	macro
	.set	reorder
	.end	boyAI_sub_1518CC
