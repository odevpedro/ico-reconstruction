	.text
	.p2align 3
	.globl	boyAI_sub_150B08
	.ent	boyAI_sub_150B08
boyAI_sub_150B08:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	slti	$2,$2,0x65
	bnez	$2,loc_00150bb0
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x96
	b	loc_00150bb0
	nop	
	addiu	$3,$0,-0x65
	addiu	$16,$0,5
	addiu	$5,$0,-5
	addiu	$2,$2,-0x80
	lw	$4,0($29)
	slt	$3,$3,$2
	slti	$2,$2,0x65
	movn	$16,$0,$3
	jal	0x0015ef88
	movz	$16,$5,$2
	lwc1	$f0,0($2)
	lw	$4,0($29)
	jal	0x0015ef88
	swc1	$f0,0x20($29)
	lwc1	$f0,4($2)
	lw	$4,0($29)
	jal	0x0015ef88
	swc1	$f0,0x24($29)
	mtc1	$16,$f12
	cvt.s.w	$f12,$f12
	lwc1	$f1,-0x7d00($28)
	lui	$1,0x4334
	mtc1	$1,$f2
	move	$4,$21
	lwc1	$f0,8($2)
	mul.s	$f12,$f12,$f1
	nop	
	nop	
	div.s	$f12,$f12,$f2
	jal	0x001945b8
	swc1	$f0,0x28($29)
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$21
loc_00150bb0:
	lw	$4,0($29)
	.word	0x10000093
	addiu	$5,$0,0x13a
	.set	macro
	.set	reorder
	.end	boyAI_sub_150B08
