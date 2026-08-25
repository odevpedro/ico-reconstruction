	.text
	.p2align 3
	.globl	boyAI_sub_150720
	.ent	boyAI_sub_150720
boyAI_sub_150720:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	lwc1	$f2,0x33c($20)
	mtc1	$0,$f0
	.word	0x46001032
	bc1t	loc_00150770
	nop	
	lw	$5,0($29)
	move	$4,$22
	jal	0x0014d6b0
	lw	$6,0x7c($29)
	beqz	$2,loc_0015076c
	lwc1	$f0,0x10($29)
	lwc1	$f2,0x33c($20)
	swc1	$f0,0x110($20)
	lwc1	$f1,0x14($29)
	swc1	$f1,0x114($20)
	lwc1	$f0,0x18($29)
	b	loc_00150770
	swc1	$f0,0x118($20)
loc_0015076c:
	lwc1	$f2,0x33c($20)
loc_00150770:
	lwc1	$f0,-0x7d04($28)
	.word	0x46020034
	nop	
	bc1f	loc_001507c4
	addiu	$2,$0,0x6f
	lw	$3,0x30($20)
	beq	$3,$2,loc_001507c4
	nop	
	lw	$2,0($29)
	lui	$3,0x56
	addiu	$6,$3,0x5060
	addiu	$7,$0,0x190
	lw	$3,0x15c($2)
	lw	$4,0($29)
	lw	$2,0x4a0($3)
	mult	$2,$2,$7
	addu	$6,$6,$2
	lwc1	$f12,0x138($6)
	cvt.s.w	$f12,$f12
	jal	0x0015b780
	lw	$5,0x7c($29)
loc_001507c4:
	lw	$4,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_150720
