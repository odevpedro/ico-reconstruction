	.text
	.p2align 3
	.globl	boyAI_sub_1507C0
	.ent	boyAI_sub_1507C0
boyAI_sub_1507C0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$5,0x7c($29)
	lw	$4,0($29)
	jal	0x00181bc8
	nop	
	lw	$4,0($29)
	jal	0x001448d8
	nop	
	lw	$4,0($29)
	jal	0x00149ef8
	nop	
	bnel	$2,$0,loc_00150800
	lw	$3,0x30($20)
	lw	$4,0($29)
	jal	0x00154cb8
	nop	
	lw	$3,0x30($20)
loc_00150800:
	sltiu	$2,$3,4
	.word	0x5040004e
	addiu	$3,$3,-1
	.word	0x1060004c
	addiu	$3,$3,-1
	lw	$2,0x2d4($20)
	andi	$2,$2,0x20
	.word	0x10400049
	sltiu	$2,$3,0x44
	lw	$18,0($29)
	jal	0x0013eb50
	addiu	$4,$0,0x13
	lui	$1,0x42c8
	mtc1	$1,$f20
	move	$17,$2
	addiu	$19,$0,0x2d
	.word	0x1220003d
	sw	$0,0x70($29)
	lw	$2,0x16c($17)
	nop	
	.word	0x10400034
	nop	
	jal	0x0015eff8
	move	$4,$18
	move	$16,$2
	jal	0x0015eff8
	move	$4,$17
	move	$4,$16
	jal	0x001943c8
	move	$5,$2
	.word	0x46140034
	.word	0x45000029
	nop	
	jal	0x0015eff8
	move	$4,$17
	move	$16,$2
	jal	0x0015eff8
	move	$4,$18
	move	$5,$16
	.set	macro
	.set	reorder
	.end	boyAI_sub_1507C0
