	.text
	.p2align 3
	.globl	boyAI_sub_156A6C
	.ent	boyAI_sub_156A6C
boyAI_sub_156A6C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$5,$0,0x9d
	bltz	$18,loc_00156b04
	nop	
	bne	$18,$22,loc_00156a94
	slt	$2,$18,$22
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$23
	b	loc_00156b00
	move	$18,$19
loc_00156a94:
	beqz	$2,loc_00156afc
	mult	$2,$30,$18
	lwc1	$f3,-0x7cc4($28)
	lui	$1,0x4334
	mtc1	$1,$f4
	lwc1	$f1,0x20($29)
	lwc1	$f2,0x24($29)
	div	$0,$2,$20
	lwc1	$f0,0x28($29)
	swc1	$f1,0x40($29)
	swc1	$f2,0x44($29)
	swc1	$f0,0x48($29)
	beql	$20,$0,loc_00156acc
	break	0,7
loc_00156acc:
	mflo	$2
	mtc1	$2,$f12
	cvt.s.w	$f12,$f12
	mul.s	$f12,$f12,$f3
	nop	
	nop	
	div.s	$f12,$f12,$f4
	jal	0x001945b8
	move	$4,$21
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$21
loc_00156afc:
	move	$18,$19
loc_00156b00:
	addiu	$19,$19,1
loc_00156b04:
	jal	0x001903e0
	lw	$4,0x180($17)
	.set	macro
	.set	reorder
	.end	boyAI_sub_156A6C
