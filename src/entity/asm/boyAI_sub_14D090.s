	.text
	.p2align 3
	.globl	boyAI_sub_14D090
	.ent	boyAI_sub_14D090
boyAI_sub_14D090:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	neg.s	$f0,$f1
	.word	0x46150034
	nop	
	bc1tl	loc_0014d0c0
	lw	$3,0x30($23)
	b	loc_0014d0e4
	addiu	$4,$29,0x30
	.word	0x46150834
	nop	
	bc1f	loc_0014d0e4
	addiu	$4,$29,0x30
	lw	$3,0x30($23)
loc_0014d0c0:
	addiu	$2,$0,4
	bne	$3,$2,loc_0014d0e4
	addiu	$4,$29,0x30
	mtc1	$0,$f0
	.word	0x46010034
	nop	
	bc1fl	loc_0014d0e4
	addiu	$30,$0,1
	addiu	$22,$0,1
loc_0014d0e4:
	move	$5,$18
	jal	0x0014a100
	addiu	$6,$0,0x12
	lwc1	$f0,0x34($29)
	lwc1	$f1,4($29)
	sw	$0,0x20($29)
	sub.s	$f0,$f0,$f1
	sw	$0,0x28($29)
	.word	0x1000000b
	swc1	$f0,0x24($29)
	jal	0x0013ebe0
	move	$4,$17
	move	$17,$2
	.word	0x5620ffa3
	lw	$2,0x16c($17)
	.word	0x10000004
	nop	
	addiu	$20,$29,0x70
	addiu	$19,$29,0x60
	.set	macro
	.set	reorder
	.end	boyAI_sub_14D090
