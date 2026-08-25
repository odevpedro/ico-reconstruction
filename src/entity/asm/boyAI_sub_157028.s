	.text
	.p2align 3
	.globl	boyAI_sub_157028
	.ent	boyAI_sub_157028
boyAI_sub_157028:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lwc1	$f0,8($2)
	nop	
	nop	
	div.s	$f12,$f12,$f1
	jal	0x001945b8
	swc1	$f0,0x68($29)
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$18
	jal	0x00203aa0
	addiu	$4,$0,1
	slt	$2,$17,$16
	.word	0x1440ffe3
	nop	
	lw	$4,0($29)
	move	$5,$19
	jal	0x00106ab8
	addiu	$16,$0,0x2b
	lw	$4,0($29)
	lui	$1,0x4120
	mtc1	$1,$f20
	jal	0x0015bcc8
	addiu	$5,$0,0x8f
	lw	$5,0($29)
	move	$4,$22
	jal	0x0014a100
	addiu	$6,$0,0x16
	lw	$5,0($29)
	move	$4,$21
	jal	0x0014a100
	addiu	$6,$0,6
	lw	$2,0x3c($29)
	lwc1	$f2,0x20($29)
	lwc1	$f0,0x24($29)
	move	$4,$2
	lwc1	$f1,0x28($29)
	lw	$3,0xc($2)
	swc1	$f0,0x94($29)
	swc1	$f2,0x90($29)
	.word	0x14700004
	swc1	$f1,0x98($29)
	lw	$5,0($29)
	jal	0x001572f0
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_157028
