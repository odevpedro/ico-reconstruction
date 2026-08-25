	.text
	.p2align 3
	.globl	girlBrain_sub_16E6C4
	.ent	girlBrain_sub_16E6C4
girlBrain_sub_16E6C4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sw	$2,0x340($7)
	lw	$4,0x90($29)
	lw	$5,0xe8($29)
	slt	$2,$5,$4
	bnez	$2,loc_0016e6f0
	addiu	$2,$0,2
	lui	$6,0x28
	addiu	$2,$6,0x2ac0
	lbu	$3,0x58e3($2)
	beqz	$3,loc_0016e760
	addiu	$2,$0,2
loc_0016e6f0:
	beq	$18,$2,loc_0016e710
	slti	$2,$18,3
	bnez	$2,loc_0016e730
	addiu	$2,$0,3
	beq	$18,$2,loc_0016e720
	nop	
	b	loc_0016e730
	nop	
loc_0016e710:
	lui	$1,0x3f00
	mtc1	$1,$f1
	b	loc_0016e734
	nop	
loc_0016e720:
	lui	$1,0x3f80
	mtc1	$1,$f1
	b	loc_0016e734
	nop	
loc_0016e730:
	mtc1	$0,$f1
loc_0016e734:
	mtc1	$0,$f0
	.word	0x46000832
	nop	
	bc1t	loc_0016e764
	lw	$7,0xec($29)
	addiu	$2,$0,1
	sw	$0,0x90($29)
	sw	$2,0x340($16)
	b	loc_0016e760
	move	$4,$0
	lw	$4,0x90($29)
loc_0016e760:
	lw	$7,0xec($29)
loc_0016e764:
	addiu	$2,$4,1
	slt	$3,$4,$7
	beqz	$3,loc_0016e79c
	sw	$2,0x90($29)
	lw	$2,0x94($29)
	move	$6,$0
	lw	$4,0($29)
	addiu	$7,$0,2
	lw	$5,0x30($29)
	addiu	$8,$0,1
	jal	0x0014b168
	sw	$0,0x33c($2)
	b	loc_0016e7c8
	lw	$4,0x94($29)
loc_0016e79c:
	lw	$4,0($29)
	move	$5,$0
	move	$6,$0
	addiu	$7,$0,6
	jal	0x0014b168
	addiu	$8,$0,1
	lw	$3,0x94($29)
	addiu	$2,$0,8
	sw	$0,0x90($29)
	sw	$2,0x340($3)
	lw	$4,0x94($29)
loc_0016e7c8:
	ori	$2,$0,0xc000
	dsll32	$2,$2,8
	ld	$3,0x18($4)
	and	$2,$3,$2
	beqz	$2,loc_0016e8fc
	nop	
	ori	$2,$0,0x8000
	dsll32	$2,$2,9
	or	$2,$3,$2
	b	loc_0016e8fc
	sd	$2,0x18($4)
	lw	$2,0x50($29)
	bnez	$2,loc_0016e888
	addiu	$2,$0,5
	b	loc_0016e888
	addiu	$2,$0,7
	lui	$8,0x27
	addiu	$3,$0,0xa
	lw	$2,0x4ec0($8)
	addiu	$4,$8,0x4ec0
	lw	$5,4($4)
	addiu	$7,$0,0x3c
	mult	$2,$2,$3
	addiu	$8,$0,0xc8
	lw	$4,0x90($29)
	beql	$5,$0,loc_0016e834
	break	0,7
loc_0016e834:
	addiu	$3,$0,0x3c
	move	$6,$4
	subu	$2,$7,$2
	addiu	$4,$4,1
	div	$0,$2,$5
	mflo	$2
	mult	$2,$2,$8
	div	$0,$2,$7
	mflo	$2
	slt	$6,$6,$2
	beqz	$6,loc_0016e884
	sw	$4,0x90($29)
	lw	$2,0x50($29)
	sw	$0,0x33c($16)
	sw	$2,0x40($16)
	lw	$4,0($29)
	jal	0x0015bcc8
	lw	$5,0x54($29)
	b	loc_0016e8fc
	nop	
loc_0016e884:
	addiu	$2,$0,8
loc_0016e888:
	sw	$0,0x90($29)
	b	loc_0016e8fc
	sw	$2,0x340($16)
	lw	$2,0x90($29)
	addiu	$3,$0,2
	addiu	$2,$2,1
	bne	$2,$3,loc_0016e8b4
	sw	$2,0x90($29)
	lw	$4,0($29)
	jal	0x0016ac10
	move	$5,$21
loc_0016e8b4:
	lui	$2,0x27
	addiu	$5,$0,0xa
	lw	$4,0x4ec0($2)
	addiu	$6,$2,0x4ec0
	lw	$3,4($6)
	addiu	$2,$0,0x3c
	.word	0x00852018
	beql	$3,$0,loc_0016e8d8
	break	0,7
loc_0016e8d8:
	lw	$5,0x90($29)
	subu	$2,$2,$4
	div	$0,$2,$3
	mflo	$2
	slt	$2,$2,$5
	beqz	$2,loc_0016e8fc
	lw	$2,0x94($29)
	sw	$0,0x90($29)
	sw	$0,0x340($2)
loc_0016e8fc:
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000fc82
	nop	
	nop	
	addiu	$29,$29,-0x160
	lui	$2,0x28
	sd	$17,0xe0($29)
	swc1	$f21,0x158($29)
	move	$17,$4
	lw	$2,0x5cf0($2)
	mov.s	$f21,$f12
	sd	$31,0x140($29)
	sd	$22,0x130($29)
	sd	$21,0x120($29)
	sd	$20,0x110($29)
	sd	$19,0x100($29)
	sd	$18,0xf0($29)
	sd	$16,0xd0($29)
	beqz	$2,loc_0016eb38
	swc1	$f20,0x150($29)
	sw	$0,0($17)
	move	$18,$0
	sw	$0,4($17)
	sw	$0,8($17)
	blez	$2,loc_0016ea10
	lwc1	$f20,0($17)
	lui	$22,0x29
	addiu	$20,$29,0x10
	addiu	$21,$29,0x20
	addiu	$16,$0,0x30
loc_0016e978:
	addiu	$5,$22,-0x7d00
	.word	0x02508018
	addiu	$6,$5,-0x25f0
	addiu	$19,$5,-0x5840
	move	$4,$29
	jal	0x00243ae8
	addu	$6,$16,$6
	sw	$0,4($29)
	move	$4,$29
	jal	0x00243978
	move	$5,$29
	addu	$16,$16,$19
	lui	$1,0x3f80
	mtc1	$1,$f1
	lwc1	$f0,0x3260($16)
	.word	0x46010034
	nop	
	bc1tl	loc_0016e9c4
	mov.s	$f0,$f1
loc_0016e9c4:
	nop	
	nop	
	div.s	$f0,$f1,$f0
	move	$4,$29
	move	$5,$29
	addiu	$18,$18,1
	mov.s	$f12,$f0
	jal	0x00243b18
	add.s	$f20,$f20,$f0
	move	$4,$17
	move	$5,$17
	jal	0x00243ad0
	move	$6,$29
	lw	$2,0x3230($19)
	slt	$2,$18,$2
	bnez	$2,loc_0016e978
	addiu	$16,$0,0x30
	b	loc_0016ea1c
	nop	
loc_0016ea10:
	lui	$22,0x29
	addiu	$20,$29,0x10
	addiu	$21,$29,0x20
loc_0016ea1c:
	mtc1	$0,$f0
	.word	0x4600a032
	nop	
	bc1t	loc_0016ea50
	lui	$16,0x56
	nop	
	nop	
	div.s	$f12,$f21,$f20
	move	$4,$17
	jal	0x00243b18
	move	$5,$17
	b	loc_0016ea78
	move	$4,$17
loc_0016ea50:
	addiu	$5,$0,0x75c
	addiu	$16,$16,-0x6ad8
	jal	0x001ad768
	move	$4,$16
	lui	$6,0x63
	move	$4,$16
	addiu	$6,$6,0x2450
	jal	0x00263ff0
	addiu	$5,$0,0x75c
	move	$4,$17
loc_0016ea78:
	jal	0x00243978
	move	$5,$17
	addiu	$16,$22,-0x7d00
	mov.s	$f12,$f21
	move	$4,$17
	jal	0x00243b18
	move	$5,$17
	move	$4,$17
	move	$5,$16
	jal	0x00243ad0
	move	$6,$17
	lwc1	$f1,-0xc($16)
	addiu	$5,$16,-0x30
	lui	$1,0x4248
	mtc1	$1,$f0
	move	$4,$20
	swc1	$f1,4($17)
	jal	0x00243b60
	swc1	$f0,0x80($29)
	move	$4,$21
	jal	0x00243b60
	move	$5,$17
	jal	0x001683c8
	move	$4,$20
	lwc1	$f0,0x34($29)
	move	$4,$20
	lui	$1,0x4348
	mtc1	$1,$f1
	lwc1	$f4,0x30($29)
	add.s	$f3,$f0,$f1
	lwc1	$f2,0x38($29)
	sub.s	$f0,$f0,$f1
	swc1	$f4,0x20($29)
	swc1	$f2,0x28($29)
	swc1	$f3,0x24($29)
	swc1	$f0,0x14($29)
	swc1	$f4,0x10($29)
	jal	0x00168538
	swc1	$f2,0x18($29)
	lwc1	$f0,0x34($29)
	lui	$1,0x4120
	mtc1	$1,$f1
	lwc1	$f3,0x30($29)
	sub.s	$f0,$f0,$f1
	lwc1	$f2,0x38($29)
	swc1	$f3,0($17)
	swc1	$f2,8($17)
	swc1	$f0,4($17)
loc_0016eb38:
	ld	$31,0x140($29)
	ld	$22,0x130($29)
	ld	$21,0x120($29)
	ld	$20,0x110($29)
	ld	$19,0x100($29)
	ld	$18,0xf0($29)
	ld	$17,0xe0($29)
	ld	$16,0xd0($29)
	lwc1	$f21,0x158($29)
	lwc1	$f20,0x150($29)
	jr	$31
	addiu	$29,$29,0x160
	addiu	$29,$29,-0x70
	sd	$17,0x30($29)
	sd	$19,0x50($29)
	sd	$18,0x40($29)
	andi	$19,$5,0xff
	lw	$17,-0x6e08($28)
	move	$18,$4
	sd	$31,0x60($29)
	move	$4,$29
	sd	$16,0x20($29)
	jal	0x00104748
	move	$5,$17
	move	$4,$29
	jal	0x00194508
	move	$5,$18
	bltzl	$2,loc_0016ebac
	negu	$2,$2
loc_0016ebac:
	slti	$2,$2,0x2e
	bnez	$2,loc_0016ec5c
	ld	$31,0x60($29)
	lw	$3,0x164($17)
	addiu	$16,$29,0x10
	lwc1	$f1,0($18)
	move	$4,$16
	lw	$2,0x678($3)
	move	$5,$17
	swc1	$f1,0x3d0($2)
	lwc1	$f0,4($18)
	swc1	$f0,0x3d4($2)
	lwc1	$f1,8($18)
	jal	0x0016a5f0
	swc1	$f1,0x3d8($2)
	move	$4,$16
	jal	0x00194508
	move	$5,$18
	blezl	$2,loc_0016ec30
	lui	$4,0x56
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x6990
	beqz	$19,loc_0016ec20
	move	$4,$17
	jal	0x0015bcc8
	addiu	$5,$0,0xdc
	b	loc_0016ec5c
	ld	$31,0x60($29)
loc_0016ec20:
	jal	0x0015bcc8
	addiu	$5,$0,0xde
	b	loc_0016ec5c
	ld	$31,0x60($29)
loc_0016ec30:
	jal	0x001a6e28
	addiu	$4,$4,-0x6980
	beqz	$19,loc_0016ec50
	move	$4,$17
	jal	0x0015bcc8
	addiu	$5,$0,0xdb
	b	loc_0016ec5c
	ld	$31,0x60($29)
loc_0016ec50:
	jal	0x0015bcc8
	addiu	$5,$0,0xdd
	ld	$31,0x60($29)
loc_0016ec5c:
	ld	$19,0x50($29)
	ld	$18,0x40($29)
	ld	$17,0x30($29)
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0x70
	nop	
	addiu	$29,$29,-0x100
	sd	$23,0xc0($29)
	sd	$22,0xb0($29)
	move	$23,$5
	sd	$20,0x90($29)
	move	$22,$4
	sd	$19,0x80($29)
	addiu	$20,$29,0x10
	move	$19,$6
	sd	$30,0xd0($29)
	sd	$16,0x50($29)
	move	$4,$20
	sd	$31,0xe0($29)
	move	$5,$22
	sd	$21,0xa0($29)
	sd	$18,0x70($29)
	move	$16,$0
	sd	$17,0x60($29)
	swc1	$f20,0xf0($29)
	sw	$2,0x40($29)
	jal	0x00243ae8
	sw	$2,0($29)
	addiu	$30,$29,0x20
	move	$5,$23
	move	$4,$30
	jal	0x00243ae8
	move	$6,$19
	lui	$2,0x28
	lw	$3,0x4a20($2)
	blez	$3,loc_0016ed4c
	move	$4,$20
	addiu	$17,$29,0x30
	lui	$21,0x28
	addiu	$5,$0,0x30
loc_0016ed00:
	addiu	$18,$21,0x4a40
	.word	0x02052818
	move	$4,$17
	move	$6,$19
	jal	0x00243ae8
	addu	$5,$5,$18
	move	$4,$20
	jal	0x00194508
	move	$5,$17
	bltzl	$2,loc_0016ed2c
	negu	$2,$2
loc_0016ed2c:
	slti	$2,$2,0x2d
	bnez	$2,loc_0016ed70
	addiu	$16,$16,1
	lw	$2,-0x20($18)
	slt	$2,$16,$2
	bnez	$2,loc_0016ed00
	addiu	$5,$0,0x30
	move	$4,$20
loc_0016ed4c:
	jal	0x00194508
	move	$5,$30
	bltzl	$2,loc_0016ed5c
	negu	$2,$2
loc_0016ed5c:
	slti	$2,$2,0x2e
	bnez	$2,loc_0016ed88
	move	$4,$22
	b	loc_0016edd4
	addiu	$2,$0,1
loc_0016ed70:
	lui	$1,0x42a0
	mtc1	$1,$f0
	addiu	$2,$0,1
	lw	$3,0x40($29)
	b	loc_0016edd4
	swc1	$f0,0x30($3)
loc_0016ed88:
	jal	0x00194360
	move	$5,$19
	mov.s	$f20,$f0
	move	$5,$19
	jal	0x00194360
	move	$4,$23
	.word	0x4600a034
	nop	
	bc1f	loc_0016edd0
	move	$4,$22
	jal	0x00194360
	move	$5,$23
	lui	$1,0x46c8
	mtc1	$1,$f1
	.word	0x46010034
	nop	
	bc1f	loc_0016edd4
	addiu	$2,$0,1
loc_0016edd0:
	move	$2,$0
loc_0016edd4:
	ld	$31,0xe0($29)
	ld	$30,0xd0($29)
	ld	$23,0xc0($29)
	ld	$22,0xb0($29)
	ld	$21,0xa0($29)
	ld	$20,0x90($29)
	ld	$19,0x80($29)
	ld	$18,0x70($29)
	ld	$17,0x60($29)
	ld	$16,0x50($29)
	lwc1	$f20,0xf0($29)
	jr	$31
	addiu	$29,$29,0x100
	addiu	$29,$29,-0x160
	lui	$1,0x42a0
	mtc1	$1,$f0
	sw	$4,0($29)
	sd	$30,0x120($29)
	addiu	$4,$0,1
	sd	$23,0x110($29)
	move	$30,$0
	sd	$22,0x100($29)
	sd	$20,0xe0($29)
	sd	$19,0xd0($29)
	swc1	$f22,0x150($29)
	addiu	$19,$0,1
	swc1	$f21,0x148($29)
	sd	$31,0x130($29)
	sd	$21,0xf0($29)
	sd	$18,0xc0($29)
	sd	$17,0xb0($29)
	sd	$16,0xa0($29)
	swc1	$f20,0x140($29)
	jal	0x00203aa0
	swc1	$f0,0x30($29)
	lwc1	$f21,-0x7b4c($28)
	lui	$3,0x28
	lui	$1,0x3f80
	mtc1	$1,$f22
	addiu	$2,$3,0x2ac0
	addiu	$22,$29,0x20
	lwc1	$f0,0x57f0($2)
	addiu	$23,$29,0x40
	lwc1	$f1,0x57f4($2)
	addiu	$20,$29,0x10
	lwc1	$f2,0x57f8($2)
	swc1	$f0,0x10($29)
	swc1	$f1,0x14($29)
	swc1	$f2,0x18($29)
	lui	$2,0x27
	nop	
loc_0016eea0:
	addiu	$5,$0,0xa
	lw	$4,0x4ec0($2)
	addiu	$2,$2,0x4ec0
	.word	0x00852018
	lw	$3,4($2)
	addiu	$2,$0,0x3c
	beql	$3,$0,loc_0016eec0
	break	0,7
loc_0016eec0:
	subu	$2,$2,$4
	div	$0,$2,$3
	mflo	$2
	sll	$2,$2,1
	div	$0,$30,$2
	mfhi	$3
	bnez	$3,loc_0016ef00
	move	$30,$19
	jal	0x0016b3c0
	move	$4,$0
	addiu	$3,$0,3
	beq	$2,$3,loc_0016ef00
	lui	$2,0x28
	addiu	$3,$0,1
	addiu	$2,$2,0x2ac0
	sb	$3,0x58e0($2)
loc_0016ef00:
	lwc1	$f12,0x30($29)
	jal	0x0016e910
	addiu	$4,$29,0x20
	jal	0x0015eff8
	lw	$4,-0x6e0c($28)
	move	$6,$2
	move	$4,$20
	move	$2,$29
	jal	0x0016ec78
	addiu	$5,$29,0x20
	beqz	$2,loc_0016efb0
	lw	$3,-0x6f60($28)
	jal	0x0015eff8
	lw	$4,-0x6e0c($28)
	lw	$4,0($29)
	jal	0x0015eff8
	move	$16,$2
	move	$4,$16
	jal	0x00194400
	move	$5,$2
	mov.s	$f1,$f0
	lui	$1,0x4348
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0016ef84
	swc1	$f1,0x30($29)
	lui	$1,0x4448
	mtc1	$1,$f0
	.word	0x46010034
	nop	
	bc1fl	loc_0016ef84
	mov.s	$f0,$f1
loc_0016ef84:
	mov.s	$f12,$f0
	swc1	$f0,0x30($29)
	jal	0x0016e910
	move	$4,$20
	lwc1	$f0,0x10($29)
	lwc1	$f1,0x14($29)
	lwc1	$f2,0x18($29)
	swc1	$f0,0x20($29)
	swc1	$f1,0x24($29)
	swc1	$f2,0x28($29)
	lw	$3,-0x6f60($28)
loc_0016efb0:
	addiu	$2,$0,8
	beq	$3,$2,loc_0016efc4
	addiu	$2,$0,0x16
	bne	$3,$2,loc_0016f008
	move	$2,$0
loc_0016efc4:
	lui	$3,0x28
	lui	$1,0x42c8
	mtc1	$1,$f0
	addiu	$2,$3,0x2ac0
	lwc1	$f2,0x24($29)
	lwc1	$f1,0x5824($2)
	add.s	$f1,$f1,$f0
	.word	0x46020834
	nop	
	bc1tl	loc_0016f008
	addiu	$2,$0,1
	lwc1	$f0,0x5844($2)
	.word	0x46000834
	nop	
	bc1t	loc_0016f008
	addiu	$2,$0,1
	move	$2,$0
loc_0016f008:
	andi	$2,$2,0xff
	bnel	$2,$0,loc_0016f160
	addiu	$19,$19,1
	lui	$2,0x29
	lui	$3,0x28
	addiu	$18,$2,-0x7d20
	addiu	$3,$3,0x2ac0
	lw	$17,0x3230($3)
	addiu	$4,$29,0x40
	move	$5,$18
	jal	0x00243ae8
	addiu	$6,$29,0x20
	addiu	$21,$18,-0x25e0
	sw	$0,0x44($29)
	addiu	$4,$29,0x50
	addiu	$5,$29,0x40
	jal	0x001940b0
	move	$16,$0
	lwc1	$f12,0x40($29)
	lwc1	$f1,0x44($29)
	mul.s	$f12,$f12,$f12
	lwc1	$f0,0x48($29)
	mul.s	$f1,$f1,$f1
	mul.s	$f0,$f0,$f0
	add.s	$f12,$f12,$f1
	jal	0x00105fe0
	add.s	$f12,$f12,$f0
	blez	$17,loc_0016f144
	mov.s	$f20,$f0
	addiu	$19,$19,1
	addiu	$5,$0,0x30
	nop	
loc_0016f088:
	lwc1	$f1,4($18)
	mult	$2,$16,$5
	addiu	$4,$29,0x90
	addiu	$6,$29,0x20
	addiu	$16,$16,1
	addu	$5,$2,$21
	lwc1	$f0,0x14($5)
	sub.s	$f0,$f0,$f1
	cvt.w.s	$f1,$f0
	mfc1	$2,$f1
	nop	
	bltzl	$2,loc_0016f0bc
	negu	$2,$2
loc_0016f0bc:
	slti	$2,$2,0xc9
	beqz	$2,loc_0016f130
	addiu	$5,$5,0x10
	jal	0x00243ae8
	nop	
	addiu	$4,$29,0x90
	sw	$0,0x94($29)
	sw	$0,0x9c($29)
	addiu	$5,$29,0x50
	jal	0x002438b8
	move	$6,$4
	lwc1	$f1,0x98($29)
	mtc1	$0,$f0
	.word	0x46010034
	nop	
	bc1f	loc_0016f134
	slt	$2,$16,$17
	.word	0x46140834
	nop	
	bc1f	loc_0016f134
	lwc1	$f0,0x90($29)
	lwc1	$f1,0x94($29)
	mul.s	$f0,$f0,$f0
	mul.s	$f1,$f1,$f1
	add.s	$f0,$f0,$f1
	.word	0x46150034
	nop	
	bc1t	loc_0016f14c
	addiu	$2,$0,1
loc_0016f130:
	slt	$2,$16,$17
loc_0016f134:
	bnez	$2,loc_0016f088
	addiu	$5,$0,0x30
	b	loc_0016f14c
	move	$2,$0
loc_0016f144:
	addiu	$19,$19,1
	move	$2,$0
loc_0016f14c:
	andi	$2,$2,0xff
	beqz	$2,loc_0016f170
	lui	$2,0x28
	b	loc_0016f168
	addiu	$3,$0,1
loc_0016f160:
	lui	$2,0x28
	addiu	$3,$0,1
loc_0016f168:
	addiu	$2,$2,0x2ac0
	sb	$3,0x58e0($2)
loc_0016f170:
	lui	$2,0x29
	move	$4,$22
	addiu	$16,$2,-0x7d20
	addiu	$17,$0,1
	jal	0x00194360
	move	$5,$16
	lui	$1,0x4561
	mtc1	$1,$f1
	.word	0x46010034
	nop	
	bc1fl	loc_0016f1a0
	move	$17,$0
loc_0016f1a0:
	beqz	$17,loc_0016f1bc
	lwc1	$f0,0x20($29)
	lwc1	$f1,0x24($29)
	lwc1	$f2,0x28($29)
	swc1	$f0,0x10($29)
	swc1	$f1,0x14($29)
	swc1	$f2,0x18($29)
loc_0016f1bc:
	move	$4,$20
	jal	0x00194360
	move	$5,$16
	.word	0x46150034
	nop	
	bc1t	loc_0016f1e0
	lw	$3,-0x6e08($28)
	beqz	$17,loc_0016f24c
	move	$6,$16
loc_0016f1e0:
	addiu	$5,$0,7
	lw	$4,0($29)
	move	$6,$0
	lw	$2,0x164($3)
	lui	$1,0xbf80
	mtc1	$1,$f12
	jal	0x0014b270
	sw	$0,0x33c($2)
	move	$4,$20
	jal	0x00194360
	move	$5,$16
	lui	$1,0x45c8
	mtc1	$1,$f1
	.word	0x46010034
	nop	
	bc1t	loc_0016f22c
	move	$6,$16
	beqz	$17,loc_0016f294
	nop	
loc_0016f22c:
	addiu	$4,$29,0x40
	jal	0x001947d0
	addiu	$5,$16,0x20
	addiu	$4,$29,0x40
	jal	0x0016eb68
	addiu	$5,$0,1
	b	loc_0016f294
	nop	
loc_0016f24c:
	move	$4,$23
	jal	0x001947d0
	move	$5,$22
	lw	$3,-0x6e08($28)
	addiu	$5,$0,6
	lw	$4,0($29)
	move	$6,$0
	lw	$2,0x164($3)
	lui	$1,0xbf80
	mtc1	$1,$f12
	swc1	$f22,0x33c($2)
	lwc1	$f0,0x40($29)
	swc1	$f0,0x110($2)
	lwc1	$f1,0x44($29)
	swc1	$f1,0x114($2)
	lwc1	$f0,0x48($29)
	jal	0x0014b270
	swc1	$f0,0x118($2)
loc_0016f294:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0016eea0
	lui	$2,0x27
	nop	
	addiu	$29,$29,-0xa0
	sd	$21,0x60($29)
	sd	$20,0x50($29)
	move	$21,$4
	sd	$18,0x30($29)
	move	$20,$5
	sd	$31,0x70($29)
	move	$18,$0
	sd	$19,0x40($29)
	sd	$17,0x20($29)
	sd	$16,0x10($29)
	swc1	$f22,0x90($29)
	swc1	$f21,0x88($29)
	swc1	$f20,0x80($29)
	blez	$6,loc_0016f39c
	sw	$2,0($29)
	lui	$2,0x6b
	mtc1	$0,$f22
	lui	$1,0x42c8
	mtc1	$1,$f20
	addiu	$19,$2,-0x4560
	lwc1	$f21,-0x7b48($28)
	move	$17,$6
	move	$16,$21
loc_0016f308:
	lwc1	$f1,4($20)
	lwc1	$f0,4($16)
	sub.s	$f0,$f1,$f0
	.word	0x46160034
	bc1f	loc_0016f33c
	nop	
	neg.s	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_0016f34c
	move	$4,$20
	b	loc_0016f368
	move	$2,$0
loc_0016f33c:
	.word	0x46140034
	nop	
	bc1f	loc_0016f364
	move	$4,$20
loc_0016f34c:
	jal	0x00194398
	move	$5,$16
	.word	0x46150034
	nop	
	bc1t	loc_0016f368
	addiu	$2,$0,1
loc_0016f364:
	move	$2,$0
loc_0016f368:
	andi	$2,$2,0xff
	bnez	$2,loc_0016f394
	addiu	$17,$17,-1
	lwc1	$f0,0($16)
	addiu	$18,$18,1
	swc1	$f0,0($19)
	lwc1	$f1,4($16)
	swc1	$f1,4($19)
	lwc1	$f0,8($16)
	swc1	$f0,8($19)
	addiu	$19,$19,0x10
loc_0016f394:
	bnez	$17,loc_0016f308
	addiu	$16,$16,0x10
loc_0016f39c:
	blez	$18,loc_0016f3dc
	move	$17,$0
	lui	$5,0x6b
loc_0016f3a8:
	sll	$3,$17,4
	addiu	$2,$5,-0x4560
	addu	$2,$3,$2
	addiu	$17,$17,1
	lwc1	$f1,0($2)
	addu	$3,$3,$21
	slt	$4,$17,$18
	swc1	$f1,0($3)
	lwc1	$f0,4($2)
	swc1	$f0,4($3)
	lwc1	$f1,8($2)
	bnez	$4,loc_0016f3a8
	swc1	$f1,8($3)
loc_0016f3dc:
	move	$2,$18
	ld	$31,0x70($29)
	ld	$21,0x60($29)
	ld	$20,0x50($29)
	ld	$19,0x40($29)
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	lwc1	$f22,0x90($29)
	lwc1	$f21,0x88($29)
	lwc1	$f20,0x80($29)
	jr	$31
	addiu	$29,$29,0xa0
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
	.word	0x1ae0002a
	move	$18,$0
	lui	$4,0x6b
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
	.word	0x4500000a
	nop	
	neg.s	$f1,$f2
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E6C4
