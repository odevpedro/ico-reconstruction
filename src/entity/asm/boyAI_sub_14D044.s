	.text
	.p2align 3
	.globl	boyAI_sub_14D044
	.ent	boyAI_sub_14D044
boyAI_sub_14D044:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x45000031
	nop	
	jal	0x0015eff8
	move	$4,$18
	move	$6,$2
	move	$4,$16
	jal	0x001947d0
	move	$5,$29
	jal	0x0015ef88
	move	$4,$18
	move	$5,$2
	jal	0x00194508
	move	$4,$16
	mtc1	$2,$f1
	cvt.s.w	$f1,$f1
	mtc1	$0,$f0
	.word	0x46000834
	bc1f	loc_0014d0ac
	nop	
	neg.s	$f0,$f1
	.word	0x46150034
	nop	
	bc1tl	loc_0014d0c0
	lw	$3,0x30($23)
	.word	0x1000000f
	addiu	$4,$29,0x30
loc_0014d0ac:
	.word	0x46150834
	nop	
	.word	0x4500000b
	addiu	$4,$29,0x30
	lw	$3,0x30($23)
loc_0014d0c0:
	addiu	$2,$0,4
	.word	0x14620007
	addiu	$4,$29,0x30
	.set	macro
	.set	reorder
	.end	boyAI_sub_14D044
