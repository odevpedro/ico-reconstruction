	.text
	.p2align 3
	.globl	eBrainTargetGenerator
	.ent	eBrainTargetGenerator
eBrainTargetGenerator:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
loc_00192380:
	beqz	$2,loc_001923f0
	addiu	$4,$29,0x20
	jal	0x00104508
	move	$5,$16
	addiu	$4,$29,0x10
	jal	0x00194360
	addiu	$5,$29,0x20
	.word	0x46170034
	nop	
	bc1f	loc_001923f0
	lwc1	$f0,0x14($29)
	lwc1	$f1,0x24($29)
	sub.s	$f0,$f0,$f22
	sub.s	$f0,$f0,$f1
	.word	0x46150034
	bc1f	loc_001923e0
	nop	
	neg.s	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_00192408
	move	$2,$0
	b	loc_001923f0
	nop	
loc_001923e0:
	.word	0x46140034
	nop	
	bc1t	loc_00192408
	move	$2,$0
loc_001923f0:
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
	bnel	$16,$0,loc_00192380
	lw	$2,0x16c($16)
	addiu	$2,$0,1
loc_00192408:
	andi	$2,$2,0xff
	bnez	$2,loc_0019242c
	addiu	$19,$19,1
	slti	$2,$19,6
	.word	0x1440ffbf
	addiu	$17,$17,0x10
	move	$5,$20
	jal	0x00104508
	addiu	$4,$29,0x10
loc_0019242c:
	lwc1	$f0,0x10($29)
	lwc1	$f2,0x14($29)
	lwc1	$f1,0x18($29)
	swc1	$f0,0($18)
	swc1	$f1,8($18)
	swc1	$f2,4($18)
	ld	$31,0x80($29)
	ld	$20,0x70($29)
	ld	$19,0x60($29)
	ld	$18,0x50($29)
	ld	$17,0x40($29)
	ld	$16,0x30($29)
	lwc1	$f23,0xa8($29)
	lwc1	$f22,0xa0($29)
	lwc1	$f21,0x98($29)
	lwc1	$f20,0x90($29)
	jr	$31
	addiu	$29,$29,0xb0
	nop	
	addiu	$29,$29,-0x50
	sd	$18,0x30($29)
	sd	$17,0x20($29)
	addiu	$18,$0,1
	sd	$31,0x40($29)
	move	$17,$4
	sd	$16,0x10($29)
	lw	$2,0x15c($17)
	lw	$4,0x800($2)
	lw	$3,0x58($4)
	beq	$3,$18,loc_00192554
	slti	$2,$3,2
	beqz	$2,loc_001924c0
	addiu	$2,$0,2
	beqz	$3,loc_001924d0
	lui	$16,0x56
	.word	0x1000003b
	addiu	$5,$0,0x1de
loc_001924c0:
	beq	$3,$2,loc_00192560
	lui	$16,0x56
	.word	0x10000037
	addiu	$5,$0,0x1de
loc_001924d0:
	sb	$18,0x3c($4)
	addiu	$3,$0,-1
	lw	$2,0x15c($17)
	lw	$16,0x800($2)
	lw	$2,0x58($16)
	beq	$2,$3,loc_00192510
	sll	$3,$2,3
	lui	$1,0xbf80
	mtc1	$1,$f0
	addu	$3,$16,$3
	sb	$0,0x34($3)
	lw	$2,0x58($16)
	sll	$2,$2,3
	addu	$2,$16,$2
	lw	$3,0x30($2)
	swc1	$f0,0($3)
loc_00192510:
	move	$4,$29
	move	$5,$0
	jal	0x002641d8
	addiu	$6,$0,0x10
	lui	$1,0x3f80
	mtc1	$1,$f0
	move	$4,$17
	jal	0x0015eff8
	swc1	$f0,0xc($29)
	lw	$4,0x38($16)
	move	$7,$2
	move	$5,$0
	addiu	$6,$0,-1
	jal	0x001e6a98
	move	$8,$29
	.word	0x1000001e
	sw	$18,0x58($16)
loc_00192554:
	lw	$2,0x38($4)
	.word	0x1000001b
	sw	$0,0($2)
loc_00192560:
	sb	$18,0x2c($4)
	addiu	$5,$0,-1
	lw	$2,0x15c($17)
	lw	$4,0x800($2)
	lw	$3,0x58($4)
	.word	0x1065000a
	.set	macro
	.set	reorder
	.end	eBrainTargetGenerator
