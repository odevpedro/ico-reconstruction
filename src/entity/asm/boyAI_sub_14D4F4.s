	.text
	.p2align 3
	.globl	boyAI_sub_14D4F4
	.ent	boyAI_sub_14D4F4
boyAI_sub_14D4F4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lwc1	$f2,0xb4($29)
	lwc1	$f0,0xc4($29)
	mtc1	$0,$f1
	sub.s	$f2,$f2,$f0
	.word	0x46011034
	bc1f	loc_0014d534
	nop	
	neg.s	$f1,$f2
	lui	$1,0x40a0
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0014d54c
	move	$6,$17
	b	loc_0014d55c
	lw	$3,0x30($23)
loc_0014d534:
	lui	$1,0x40a0
	mtc1	$1,$f0
	.word	0x46001034
	nop	
	bc1f	loc_0014d558
	move	$6,$17
loc_0014d54c:
	move	$4,$18
	jal	0x0013ff88
	addiu	$5,$0,0x14
loc_0014d558:
	lw	$3,0x30($23)
loc_0014d55c:
	addiu	$2,$0,0x41
	.word	0x14620046
	ld	$31,0x1c0($29)
	lwc1	$f20,-0x7dcc($28)
	jal	0x0015eff8
	move	$4,$18
	move	$19,$0
	lwc1	$f0,0($2)
	move	$4,$18
	jal	0x0015eff8
	swc1	$f0,0xf0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14D4F4
