	.text
	.p2align 3
	.globl	boyAI_sub_14D3F8
	.ent	boyAI_sub_14D3F8
boyAI_sub_14D3F8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x0013ff88
	move	$6,$17
	beqz	$30,loc_0014d4bc
	addiu	$4,$29,0xc0
	move	$5,$18
	jal	0x0014a100
	addiu	$6,$0,6
	lwc1	$f1,0x60($29)
	mtc1	$0,$f0
	.word	0x46000834
	bc1f	loc_0014d44c
	nop	
	neg.s	$f1,$f1
	lui	$1,0x4270
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0014d464
	lwc1	$f1,0x68($29)
	b	loc_0014d4bc
	nop	
loc_0014d44c:
	lui	$1,0x4270
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_0014d4bc
	lwc1	$f1,0x68($29)
loc_0014d464:
	mtc1	$0,$f0
	.word	0x46000834
	bc1f	loc_0014d498
	nop	
	neg.s	$f1,$f1
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0014d4b0
	move	$4,$18
	b	loc_0014d4bc
	nop	
loc_0014d498:
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_0014d4bc
	move	$4,$18
loc_0014d4b0:
	addiu	$5,$0,0x14
	jal	0x0013ff88
	move	$6,$17
loc_0014d4bc:
	.word	0x12200026
	addiu	$2,$0,0x41
	lw	$3,0x30($23)
	.word	0x1462006c
	ld	$31,0x1c0($29)
	addiu	$16,$29,0xb0
	move	$5,$18
	move	$4,$16
	jal	0x0014a100
	addiu	$6,$0,6
	move	$6,$16
	addiu	$4,$29,0xc0
	.set	macro
	.set	reorder
	.end	boyAI_sub_14D3F8
