	.text
	.p2align 3
	.globl	boyAI_sub_14C830
	.ent	boyAI_sub_14C830
boyAI_sub_14C830:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$4,$29,0x10
	move	$4,$17
	jal	0x00194508
	addiu	$5,$29,0x10
	.word	0x0441000c
	move	$4,$17
	jal	0x00194508
	addiu	$5,$29,0x10
	negu	$2,$2
	.set	macro
	.set	reorder
	.end	boyAI_sub_14C830
