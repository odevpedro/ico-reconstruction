	.text
	.p2align 3
	.globl	boyAI_sub_1624A0
	.ent	boyAI_sub_1624A0
boyAI_sub_1624A0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
loc_001624a0:
	lw	$4,0($29)
	jal	0x00197a38
	nop	
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ff1e
	lw	$3,-0x6e08($28)
	jal	0x0013f878
	move	$4,$16
	ld	$2,0x20($17)
	addiu	$3,$0,-0x401
	and	$2,$2,$3
	b	loc_001624a0
	sd	$2,0x20($17)
	addiu	$29,$29,-0x80
	sd	$18,0x50($29)
	sd	$19,0x60($29)
	lw	$18,-0x6e0c($28)
	move	$19,$4
	sd	$31,0x70($29)
	sd	$17,0x40($29)
	.word	0x1240006f
	sd	$16,0x30($29)
	jal	0x0015eff8
	move	$4,$18
	move	$16,$2
	jal	0x0015eff8
	move	$4,$19
	move	$4,$16
	jal	0x001943c8
	move	$5,$2
	lui	$1,0x4348
	mtc1	$1,$f1
	.word	0x46010034
	nop	
	.word	0x45000061
	move	$4,$29
	jal	0x00104508
	move	$5,$19
	addiu	$16,$29,0x10
	.set	macro
	.set	reorder
	.end	boyAI_sub_1624A0
