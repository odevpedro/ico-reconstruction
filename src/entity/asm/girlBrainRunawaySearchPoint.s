	.text
	.p2align 3
	.globl	girlBrainRunawaySearchPoint
	.ent	girlBrainRunawaySearchPoint
girlBrainRunawaySearchPoint:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x1a0
	lw	$2,-0x6e08($28)
	sd	$30,0x160($29)
	lui	$3,0x6b
	sd	$23,0x150($29)
	move	$30,$4
	sd	$21,0x130($29)
	sd	$18,0x100($29)
	move	$21,$6
	sd	$16,0xe0($29)
	move	$7,$21
	sd	$31,0x170($29)
	addiu	$16,$3,-0x44c0
	sd	$22,0x140($29)
	move	$4,$16
	sd	$20,0x120($29)
	sd	$19,0x110($29)
	sd	$17,0xf0($29)
	swc1	$f22,0x190($29)
	swc1	$f21,0x188($29)
	swc1	$f20,0x180($29)
	lw	$2,0x164($2)
	sw	$5,0xd0($29)
	sw	$2,0xd4($29)
	addiu	$5,$0,0xa
	addiu	$2,$2,0x350
	sw	$2,0xd8($29)
	jal	0x001782b0
	move	$6,$2
	move	$23,$2
	addiu	$18,$23,-1
	bltz	$18,loc_0016f558
	sll	$2,$18,4
	move	$17,$16
	mtc1	$0,$f22
	addu	$16,$2,$17
	lui	$1,0x42c8
	mtc1	$1,$f20
	lwc1	$f21,-0x7b44($28)
	nop	
loc_0016f4b0:
	lwc1	$f1,4($21)
	lwc1	$f0,4($16)
	sub.s	$f0,$f1,$f0
	.word	0x46160034
	bc1f	loc_0016f4e4
	nop	
	neg.s	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_0016f4f4
	move	$4,$21
	b	loc_0016f510
	move	$2,$0
loc_0016f4e4:
	.word	0x46140034
	nop	
	bc1f	loc_0016f50c
	move	$4,$21
loc_0016f4f4:
	jal	0x00194398
	move	$5,$16
	.word	0x46150034
	nop	
	bc1t	loc_0016f510
	addiu	$2,$0,1
loc_0016f50c:
	move	$2,$0
loc_0016f510:
	andi	$2,$2,0xff
	beqz	$2,loc_0016f54c
	move	$4,$17
	lwc1	$f0,0($16)
	lw	$6,0xd8($29)
	addiu	$5,$0,0xa
	swc1	$f0,0($21)
	move	$7,$21
	lwc1	$f0,4($16)
	swc1	$f0,4($21)
	lwc1	$f1,8($16)
	jal	0x001782b0
	swc1	$f1,8($21)
	b	loc_0016f558
	move	$23,$2
loc_0016f54c:
	addiu	$18,$18,-1
	bgez	$18,loc_0016f4b0
	addiu	$16,$16,-0x10
loc_0016f558:
	lui	$3,0x6b
	move	$6,$23
	move	$2,$29
	addiu	$4,$3,-0x44c0
	jal	0x0016f2a8
	move	$5,$21
	move	$23,$2
	blez	$23,loc_0016f620
	move	$18,$0
	lui	$4,0x6b
loc_0016f580:
	sll	$3,$18,4
	addiu	$2,$4,-0x44c0
	addu	$16,$3,$2
	move	$4,$21
	jal	0x001943c8
	move	$5,$16
	lui	$2,0x28
	move	$5,$16
	jal	0x001943c8
	addiu	$4,$2,0x4a40
	lwc1	$f2,4($21)
	lwc1	$f0,4($16)
	mtc1	$0,$f1
	sub.s	$f2,$f2,$f0
	.word	0x46011034
	bc1f	loc_0016f5e8
	nop	
	neg.s	$f1,$f2
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0016f600
	move	$5,$16
	b	loc_0016f614
	addiu	$18,$18,1
loc_0016f5e8:
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46001034
	nop	
	bc1f	loc_0016f610
	move	$5,$16
loc_0016f600:
	jal	0x00194398
	move	$4,$21
	lwc1	$f1,-0x7b40($28)
	.word	0x46010034
loc_0016f610:
	addiu	$18,$18,1
loc_0016f614:
	slt	$2,$18,$23
	bnel	$2,$0,loc_0016f580
	lui	$4,0x6b
loc_0016f620:
	.word	0x1ae00054
	move	$18,$0
	lui	$3,0x6b
	nop	
	sll	$22,$18,4
	addiu	$2,$3,-0x44c0
	move	$5,$21
	addu	$19,$22,$2
	move	$17,$0
	jal	0x00194398
	move	$4,$19
	lui	$4,0x28
	addiu	$2,$4,0x2ac0
	lw	$3,0x1f60($2)
	.word	0x18600015
	mov.s	$f20,$f0
	addiu	$20,$18,1
	addiu	$5,$0,0x30
	lui	$2,0x28
	.set	macro
	.set	reorder
	.end	girlBrainRunawaySearchPoint
