	.text
	.p2align 3
	.globl	eBrainGetTarget
	.ent	eBrainGetTarget
eBrainGetTarget:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0xc0
	lui	$3,0x6d
	sd	$20,0x70($29)
	sd	$31,0xb0($29)
	move	$20,$4
	sd	$23,0xa0($29)
	addiu	$4,$3,0x710
	sd	$22,0x90($29)
	sd	$21,0x80($29)
	sd	$19,0x60($29)
	sd	$18,0x50($29)
	sd	$17,0x40($29)
	sd	$16,0x30($29)
	lw	$2,0x18($4)
	beq	$2,$20,loc_00190f90
	move	$5,$0
	addiu	$4,$4,0x18
	addiu	$5,$5,1
loc_00190f78:
	slti	$2,$5,0x20
	beqz	$2,loc_00190f90
	addiu	$4,$4,0x1c
	lw	$2,0($4)
	bnel	$2,$20,loc_00190f78
	addiu	$5,$5,1
loc_00190f90:
	addiu	$2,$0,0x20
	bne	$5,$2,loc_00190fa4
	addiu	$2,$0,0x1c
	b	loc_00190fb0
	move	$18,$0
loc_00190fa4:
	addiu	$3,$3,0x710
	mult	$2,$5,$2
	addu	$18,$2,$3
loc_00190fb0:
	beqz	$18,loc_00191878
	move	$2,$0
	lw	$2,0x10($18)
	addiu	$4,$2,-1
	sltiu	$3,$4,7
	beqz	$3,loc_00191198
	lui	$2,0x56
	sll	$3,$4,2
	addiu	$2,$2,-0x51c0
	addu	$3,$3,$2
	lw	$4,0($3)
	jr	$4
	nop	
	lhu	$4,0($18)
	addiu	$5,$0,1
	beq	$4,$5,loc_00191004
	addiu	$2,$0,2
	beq	$4,$2,loc_00191014
	lw	$2,-0x617c($28)
	b	loc_00191028
	addiu	$3,$0,1
loc_00191004:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_00191020
	sw	$2,-0x617c($28)
loc_00191014:
	lw	$2,-0x6178($28)
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_00191020:
	lw	$2,-0x617c($28)
	addiu	$3,$0,1
loc_00191028:
	addiu	$2,$2,1
	beq	$4,$3,loc_00191038
	sw	$2,-0x617c($28)
	sw	$0,0x14($18)
loc_00191038:
	b	loc_00191198
	sh	$5,0($18)
	lhu	$3,0($18)
	addiu	$2,$0,1
	beq	$3,$2,loc_00191060
	addiu	$4,$0,2
	beq	$3,$4,loc_00191070
	lw	$2,-0x6178($28)
	b	loc_00191080
	sh	$4,0($18)
loc_00191060:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_00191078
	sw	$2,-0x617c($28)
loc_00191070:
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_00191078:
	lw	$2,-0x6178($28)
	sh	$4,0($18)
loc_00191080:
	addiu	$2,$2,1
	b	loc_00191198
	sw	$2,-0x6178($28)
	lhu	$3,0($18)
	addiu	$2,$0,1
	beq	$3,$2,loc_001910ac
	addiu	$2,$0,2
	beq	$3,$2,loc_001910bc
	lw	$2,-0x6178($28)
	b	loc_00191194
	addiu	$2,$0,5
loc_001910ac:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_001910c4
	sw	$2,-0x617c($28)
loc_001910bc:
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_001910c4:
	b	loc_00191194
	addiu	$2,$0,5
	lhu	$3,0($18)
	addiu	$2,$0,1
	beq	$3,$2,loc_001910ec
	addiu	$2,$0,2
	beq	$3,$2,loc_001910fc
	lw	$2,-0x6178($28)
	b	loc_00191194
	move	$2,$0
loc_001910ec:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_00191104
	sw	$2,-0x617c($28)
loc_001910fc:
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_00191104:
	b	loc_00191194
	move	$2,$0
	lw	$2,-0x6e08($28)
	beqz	$2,loc_00191198
	addiu	$2,$0,1
	lhu	$3,0($18)
	beq	$3,$2,loc_00191134
	addiu	$2,$0,2
	beq	$3,$2,loc_00191144
	lw	$2,-0x6178($28)
	b	loc_00191194
	addiu	$2,$0,3
loc_00191134:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_0019114c
	sw	$2,-0x617c($28)
loc_00191144:
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_0019114c:
	b	loc_00191194
	addiu	$2,$0,3
	lhu	$3,0($18)
	addiu	$2,$0,1
	beq	$3,$2,loc_00191174
	addiu	$2,$0,2
	beq	$3,$2,loc_00191184
	addiu	$2,$0,4
	b	loc_00191198
	sh	$2,0($18)
loc_00191174:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_00191190
	sw	$2,-0x617c($28)
loc_00191184:
	lw	$2,-0x6178($28)
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_00191190:
	addiu	$2,$0,4
loc_00191194:
	sh	$2,0($18)
loc_00191198:
	lw	$2,-0x4b40($28)
	beqz	$2,loc_001911f8
	sw	$0,0x10($18)
	lhu	$4,0($18)
	addiu	$2,$0,1
	bnel	$4,$2,loc_001911fc
	lhu	$5,0($18)
	move	$3,$4
	beq	$3,$4,loc_001911d0
	addiu	$2,$0,2
	beq	$3,$2,loc_001911e0
	addiu	$2,$0,8
	b	loc_001911f8
	sh	$2,0($18)
loc_001911d0:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_001911ec
	sw	$2,-0x617c($28)
loc_001911e0:
	lw	$2,-0x6178($28)
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_001911ec:
	addiu	$2,$0,8
	sh	$2,0($18)
	nop	
loc_001911f8:
	lhu	$5,0($18)
loc_001911fc:
	move	$23,$0
	sltiu	$2,$5,9
	beqz	$2,loc_0019180c
	move	$6,$5
	lui	$2,0x56
	sll	$3,$5,2
	addiu	$2,$2,-0x51a0
	addu	$3,$3,$2
	lw	$4,0($3)
	jr	$4
	nop	
	lw	$8,-0x4b4c($28)
	move	$7,$0
	move	$5,$0
	blez	$8,loc_0019128c
	move	$4,$0
	lui	$9,0x6d
	lw	$2,0xa90($9)
	bne	$18,$2,loc_00191258
	lw	$6,-0x4b48($28)
	b	loc_00191290
	addiu	$7,$0,1
	nop	
loc_00191258:
	addiu	$3,$5,1
loc_0019125c:
	addiu	$4,$4,1
	movn	$5,$3,$2
	slt	$2,$4,$8
	beqz	$2,loc_00191290
	addiu	$3,$9,0xa90
	sll	$2,$4,2
	addu	$2,$2,$3
	lw	$2,0($2)
	bne	$18,$2,loc_0019125c
	addiu	$3,$5,1
	b	loc_00191290
	addiu	$7,$0,1
loc_0019128c:
	lw	$6,-0x4b48($28)
loc_00191290:
	beqz	$7,loc_001912ac
	addiu	$22,$0,-1
	lw	$2,-0x617c($28)
	addiu	$3,$0,0x1f
	addu	$2,$5,$2
	slt	$3,$3,$2
	movz	$22,$4,$3
loc_001912ac:
	move	$7,$0
	move	$5,$0
	blez	$6,loc_001912f4
	move	$4,$0
	lui	$8,0x6d
	b	loc_001912e8
	lw	$2,0xb10($8)
loc_001912c8:
	addiu	$4,$4,1
	movn	$5,$3,$2
	slt	$2,$4,$6
	beqz	$2,loc_001912f4
	addiu	$3,$8,0xb10
	sll	$2,$4,2
	addu	$2,$2,$3
	lw	$2,0($2)
loc_001912e8:
	bne	$18,$2,loc_001912c8
	addiu	$3,$5,1
	addiu	$7,$0,1
loc_001912f4:
	beqz	$7,loc_00191310
	addiu	$21,$0,-1
	lw	$2,-0x6178($28)
	addiu	$3,$0,0x1f
	addu	$2,$5,$2
	slt	$3,$3,$2
	movz	$21,$4,$3
loc_00191310:
	move	$4,$29
	move	$5,$0
	jal	0x002641d8
	addiu	$6,$0,0xc
	bltz	$22,loc_00191390
	move	$6,$0
	bltz	$21,loc_00191388
	lui	$4,0x6d
	lui	$5,0x6d
	addiu	$4,$4,0xa90
	addiu	$5,$5,0xb10
	sll	$2,$22,2
	sll	$3,$21,2
	addu	$2,$2,$4
	addu	$3,$3,$5
	lw	$4,0($2)
	lw	$5,0($3)
	lwc1	$f1,8($4)
	lwc1	$f0,0xc($5)
	.word	0x46000834
	nop	
	bc1f	loc_00191374
	addiu	$2,$0,1
	b	loc_0019137c
	addiu	$3,$0,2
loc_00191374:
	addiu	$2,$0,2
	addiu	$3,$0,1
loc_0019137c:
	sw	$2,0($29)
	b	loc_0019139c
	sw	$3,4($29)
loc_00191388:
	b	loc_00191398
	addiu	$2,$0,1
loc_00191390:
	bltz	$21,loc_0019139c
	addiu	$2,$0,2
loc_00191398:
	sw	$2,0($29)
loc_0019139c:
	sll	$2,$6,2
	addu	$2,$29,$2
	lw	$3,0($2)
	beql	$3,$0,loc_00191508
	lhu	$2,0($18)
	move	$17,$2
	addiu	$19,$0,1
loc_001913b8:
	lw	$2,0($17)
	bne	$2,$19,loc_00191464
	lw	$16,-0x6e08($28)
	lw	$16,-0x6e0c($28)
	bnez	$16,loc_001913d8
	addiu	$4,$29,0x10
	b	loc_00191408
	move	$2,$0
loc_001913d8:
	jal	0x00104508
	move	$5,$20
	addiu	$4,$29,0x20
	jal	0x00104508
	move	$5,$16
	lui	$1,0x42c8
	mtc1	$1,$f12
	move	$5,$16
	move	$4,$20
	addiu	$6,$29,0x20
	jal	0x0014ac78
	addiu	$7,$0,0xb4
loc_00191408:
	beqz	$2,loc_001914f4
	addiu	$4,$0,1
	lhu	$3,0($18)
	beq	$3,$4,loc_0019142c
	addiu	$2,$0,2
	beq	$3,$2,loc_0019143c
	lw	$2,-0x617c($28)
	b	loc_00191450
	addiu	$2,$2,1
loc_0019142c:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_00191448
	sw	$2,-0x617c($28)
loc_0019143c:
	lw	$2,-0x6178($28)
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_00191448:
	lw	$2,-0x617c($28)
	addiu	$2,$2,1
loc_00191450:
	beq	$3,$19,loc_0019145c
	sw	$2,-0x617c($28)
	sw	$0,0x14($18)
loc_0019145c:
	b	loc_00191504
	sh	$4,0($18)
loc_00191464:
	bnez	$16,loc_00191474
	addiu	$4,$29,0x10
	b	loc_001914a4
	move	$2,$0
loc_00191474:
	jal	0x00104508
	move	$5,$20
	addiu	$4,$29,0x20
	jal	0x00104508
	move	$5,$16
	lui	$1,0x42c8
	mtc1	$1,$f12
	move	$5,$16
	move	$4,$20
	addiu	$6,$29,0x20
	jal	0x0014ac78
	addiu	$7,$0,0xb4
loc_001914a4:
	beql	$2,$0,loc_001914f8
	addiu	$17,$17,4
	lhu	$2,0($18)
	beq	$2,$19,loc_001914c8
	addiu	$3,$0,2
	beq	$2,$3,loc_001914d8
	lw	$2,-0x6178($28)
	b	loc_001914e8
	sh	$3,0($18)
loc_001914c8:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_001914e0
	sw	$2,-0x617c($28)
loc_001914d8:
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_001914e0:
	lw	$2,-0x6178($28)
	sh	$3,0($18)
loc_001914e8:
	addiu	$2,$2,1
	b	loc_00191504
	sw	$2,-0x6178($28)
loc_001914f4:
	addiu	$17,$17,4
loc_001914f8:
	lw	$2,0($17)
	bnez	$2,loc_001913b8
	nop	
loc_00191504:
	lhu	$2,0($18)
loc_00191508:
	beqz	$2,loc_0019180c
	lui	$4,0x6d
	lui	$3,0x6d
	addiu	$4,$4,0xb10
	sll	$2,$21,2
	addiu	$3,$3,0xa90
	sll	$5,$22,2
	addu	$2,$2,$4
	addu	$5,$5,$3
	sw	$0,0($2)
	addiu	$23,$0,1
	b	loc_0019180c
	sw	$0,0($5)
	lw	$3,-0x6e0c($28)
	lw	$2,0x14($18)
	slti	$2,$2,0xb5
	bnez	$2,loc_0019180c
	sw	$3,4($18)
	lwc1	$f0,8($18)
	lwc1	$f1,-0x7954($28)
	lwc1	$f2,0xc($18)
	add.s	$f0,$f0,$f1
	.word	0x46001034
	nop	
	bc1f	loc_0019180c
	lw	$16,-0x6e08($28)
	bnez	$16,loc_00191580
	move	$4,$29
	b	loc_001915b0
	move	$2,$0
loc_00191580:
	jal	0x00104508
	move	$5,$20
	addiu	$4,$29,0x10
	jal	0x00104508
	move	$5,$16
	lui	$1,0x42c8
	mtc1	$1,$f12
	move	$5,$16
	move	$4,$20
	addiu	$6,$29,0x10
	jal	0x0014ac78
	addiu	$7,$0,0xb4
loc_001915b0:
	beqz	$2,loc_0019180c
	addiu	$2,$0,1
	lhu	$3,0($18)
	beq	$3,$2,loc_00191790
	addiu	$4,$0,2
	bne	$3,$4,loc_001917ac
	lw	$2,-0x6178($28)
	b	loc_001917a4
	addiu	$2,$2,-1
	lw	$2,-0x6e08($28)
	b	loc_0019180c
	sw	$2,4($18)
	lwc1	$f0,8($18)
	lwc1	$f1,-0x7950($28)
	lw	$2,-0x6e08($28)
	.word	0x46010034
	nop	
	bc1f	loc_001916a0
	sw	$2,4($18)
	lw	$16,-0x6e0c($28)
	bnez	$16,loc_00191610
	move	$4,$29
	b	loc_00191640
	move	$2,$0
loc_00191610:
	jal	0x00104508
	move	$5,$20
	addiu	$4,$29,0x10
	jal	0x00104508
	move	$5,$16
	lui	$1,0x42c8
	mtc1	$1,$f12
	move	$5,$16
	move	$4,$20
	addiu	$6,$29,0x10
	jal	0x0014ac78
	addiu	$7,$0,0xb4
loc_00191640:
	beqz	$2,loc_001916a0
	addiu	$5,$0,1
	lhu	$4,0($18)
	beq	$4,$5,loc_00191664
	addiu	$2,$0,2
	beq	$4,$2,loc_00191674
	lw	$2,-0x617c($28)
	b	loc_00191688
	addiu	$3,$0,1
loc_00191664:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_00191680
	sw	$2,-0x617c($28)
loc_00191674:
	lw	$2,-0x6178($28)
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_00191680:
	lw	$2,-0x617c($28)
	addiu	$3,$0,1
loc_00191688:
	addiu	$2,$2,1
	beq	$4,$3,loc_00191698
	sw	$2,-0x617c($28)
	sw	$0,0x14($18)
loc_00191698:
	sh	$5,0($18)
	addiu	$23,$0,1
loc_001916a0:
	lw	$2,-0x4b44($28)
	bnez	$2,loc_0019180c
	addiu	$5,$0,1
	lhu	$4,0($18)
	beq	$4,$5,loc_001916c8
	addiu	$2,$0,2
	beq	$4,$2,loc_001916d8
	lw	$2,-0x617c($28)
	b	loc_001916ec
	addiu	$3,$0,1
loc_001916c8:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_001916e4
	sw	$2,-0x617c($28)
loc_001916d8:
	lw	$2,-0x6178($28)
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_001916e4:
	lw	$2,-0x617c($28)
	addiu	$3,$0,1
loc_001916ec:
	addiu	$2,$2,1
	beq	$4,$3,loc_001916fc
	sw	$2,-0x617c($28)
	sw	$0,0x14($18)
loc_001916fc:
	sh	$5,0($18)
	b	loc_0019180c
	addiu	$23,$0,1
	jal	0x00190d70
	lw	$4,8($20)
	jal	0x0013eae8
	move	$4,$2
	b	loc_0019180c
	sw	$2,4($18)
	b	loc_0019180c
	sw	$0,4($18)
	lw	$16,-0x6e08($28)
	bnez	$16,loc_0019173c
	move	$4,$29
	b	loc_0019176c
	move	$2,$0
loc_0019173c:
	jal	0x00104508
	move	$5,$20
	addiu	$4,$29,0x10
	jal	0x00104508
	move	$5,$16
	lui	$1,0x42c8
	mtc1	$1,$f12
	move	$5,$16
	move	$4,$20
	addiu	$6,$29,0x10
	jal	0x0014ac78
	addiu	$7,$0,0xb4
loc_0019176c:
	beqz	$2,loc_0019180c
	addiu	$2,$0,1
	lhu	$3,0($18)
	beq	$3,$2,loc_00191790
	addiu	$4,$0,2
	beq	$3,$4,loc_001917a0
	lw	$2,-0x6178($28)
	b	loc_001917b0
	addiu	$23,$0,1
loc_00191790:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_001917a8
	sw	$2,-0x617c($28)
loc_001917a0:
	addiu	$2,$2,-1
loc_001917a4:
	sw	$2,-0x6178($28)
loc_001917a8:
	lw	$2,-0x6178($28)
loc_001917ac:
	addiu	$23,$0,1
loc_001917b0:
	sh	$4,0($18)
	addiu	$2,$2,1
	b	loc_0019180c
	sw	$2,-0x6178($28)
	lw	$2,-0x6e0c($28)
	lw	$3,-0x4b40($28)
	bnez	$3,loc_0019180c
	sw	$2,4($18)
	addiu	$2,$0,1
	beq	$6,$2,loc_001917f0
	move	$3,$0
	addiu	$2,$0,2
	beq	$5,$2,loc_00191800
	lw	$2,-0x6178($28)
	b	loc_0019180c
	sh	$3,0($18)
loc_001917f0:
	lw	$2,-0x617c($28)
	addiu	$2,$2,-1
	b	loc_00191808
	sw	$2,-0x617c($28)
loc_00191800:
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_00191808:
	sh	$3,0($18)
loc_0019180c:
	bnel	$23,$0,loc_001911fc
	lhu	$5,0($18)
	lw	$2,-0x4b44($28)
	beql	$2,$0,loc_00191878
	move	$2,$18
	beq	$2,$20,loc_00191874
	addiu	$2,$0,2
	lhu	$4,0($18)
	bne	$4,$2,loc_00191878
	move	$2,$18
	move	$3,$4
	addiu	$2,$0,1
	beq	$3,$2,loc_00191854
	lw	$2,-0x617c($28)
	beq	$3,$4,loc_00191860
	addiu	$2,$0,5
	b	loc_00191874
	sh	$2,0($18)
loc_00191854:
	addiu	$2,$2,-1
	b	loc_0019186c
	sw	$2,-0x617c($28)
loc_00191860:
	lw	$2,-0x6178($28)
	addiu	$2,$2,-1
	sw	$2,-0x6178($28)
loc_0019186c:
	addiu	$2,$0,5
	sh	$2,0($18)
loc_00191874:
	move	$2,$18
loc_00191878:
	ld	$31,0xb0($29)
	ld	$23,0xa0($29)
	ld	$22,0x90($29)
	ld	$21,0x80($29)
	ld	$20,0x70($29)
	ld	$19,0x60($29)
	ld	$18,0x50($29)
	ld	$17,0x40($29)
	ld	$16,0x30($29)
	jr	$31
	addiu	$29,$29,0xc0
	nop	
	lui	$2,0x6d
	sw	$0,-0x6178($28)
	addiu	$2,$2,0x710
	sw	$0,-0x617c($28)
	sw	$0,-0x4b44($28)
	addiu	$2,$2,0x37c
	sw	$0,-0x4b40($28)
	addiu	$3,$0,0x1f
loc_001918c8:
	sw	$0,0($2)
	addiu	$3,$3,-1
	addiu	$2,$2,-0x1c
	nop	
	nop	
	bgez	$3,loc_001918c8
	nop	
	jr	$31
	nop	
	nop	
	.set	macro
	.set	reorder
	.end	eBrainGetTarget
