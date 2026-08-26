	.text
	.p2align 3
	.globl	actBoyWalk
	.ent	actBoyWalk
actBoyWalk:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0xa0
	sw	$4,0($29)
	sd	$21,0x80($29)
	sd	$19,0x60($29)
	sd	$18,0x50($29)
	lui	$19,0x27
	sd	$17,0x40($29)
	addiu	$21,$19,0x4ec0
	sd	$16,0x30($29)
	addiu	$18,$29,0x20
	sd	$31,0x90($29)
	addiu	$17,$29,0x10
	lw	$2,0($29)
	addiu	$16,$0,0x3c
	sd	$20,0x70($29)
	lw	$20,0x164($2)
loc_001518a8:
	jal	0x0014a0d8
	nop
	move	$4,$18
	beqz	$2,loc_001519c4
	addiu	$6,$0,2
	jal	0x0014a100
	lw	$5,-0x6e0c($28)
	lw	$5,-0x6e08($28)
	move	$4,$17
	jal	0x0014a100
	addiu	$6,$0,0x12
	move	$4,$17
	jal	0x001943c8
	move	$5,$18
	lw	$2,0x4ec0($19)
	addiu	$3,$0,0xa
	lw	$4,4($21)
	addiu	$5,$0,0x64
	mult	$2,$2,$3
	lui	$1,0x42a0
	mtc1	$1,$f2
	beql	$4,$0,loc_00151904
	break	0,7
loc_00151904:
	mov.s	$f1,$f0
	.word	0x46011034
	lw	$3,0x48($20)
	subu	$2,$16,$2
	div	$0,$2,$4
	mflo	$2
	mult	$2,$2,$5
	div	$0,$2,$16
	mflo	$2
	slt	$2,$2,$3
	beqz	$2,loc_001519c4
	nop
	lui	$1,0x4120
	mtc1	$1,$f3
	bc1f	loc_001519c4
	nop
	sub.s	$f1,$f1,$f2
	mtc1	$0,$f0
	lui	$1,0x3f80
	mtc1	$1,$f2
	nop
	nop
	div.s	$f1,$f1,$f3
	.word	0x46000834
	nop
	bc1t	loc_00151984
	mov.s	$f12,$f0
	.word	0x46011034
	nop
	bc1f	loc_00151984
	mov.s	$f12,$f1
	mov.s	$f12,$f2
loc_00151984:
	jal	0x00263fb0
	nop
	lui	$1,0x56
	ld	$5,-0x79e0($1)
	jal	0x00262be8
	move	$4,$2
	lui	$1,0x56
	ld	$4,-0x79d8($1)
	jal	0x00262b80
	move	$5,$2
	jal	0x002633b8
	move	$4,$2
	lw	$4,0($29)
	mov.s	$f12,$f0
	jal	0x0014b330
	addiu	$5,$0,2
loc_001519c4:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_001518a8
	nop
	nop
	addiu	$29,$29,-0xa0
	sw	$4,0($29)
	sd	$21,0x80($29)
	sd	$19,0x60($29)
	sd	$18,0x50($29)
	lui	$19,0x27
	sd	$17,0x40($29)
	addiu	$21,$19,0x4ec0
	sd	$16,0x30($29)
	addiu	$18,$29,0x20
	sd	$31,0x90($29)
	addiu	$17,$29,0x10
	lw	$2,0($29)
	addiu	$16,$0,0x3c
	sd	$20,0x70($29)
	lw	$20,0x164($2)
loc_00151a18:
	jal	0x0014a0d8
	nop
	move	$4,$18
	beqz	$2,loc_00151b34
	addiu	$6,$0,2
	jal	0x0014a100
	lw	$5,-0x6e0c($28)
	lw	$5,-0x6e08($28)
	move	$4,$17
	jal	0x0014a100
	addiu	$6,$0,0x12
	move	$4,$17
	jal	0x001943c8
	move	$5,$18
	lw	$2,0x4ec0($19)
	addiu	$3,$0,0xa
	lw	$4,4($21)
	addiu	$5,$0,0x64
	mult	$2,$2,$3
	lui	$1,0x42b4
	mtc1	$1,$f2
	beql	$4,$0,loc_00151a74
	break	0,7
loc_00151a74:
	mov.s	$f1,$f0
	.word	0x46011034
	lw	$3,0x48($20)
	subu	$2,$16,$2
	div	$0,$2,$4
	mflo	$2
	mult	$2,$2,$5
	div	$0,$2,$16
	mflo	$2
	slt	$2,$2,$3
	beqz	$2,loc_00151b34
	nop
	lui	$1,0x4120
	mtc1	$1,$f3
	bc1f	loc_00151b34
	nop
	sub.s	$f1,$f1,$f2
	mtc1	$0,$f0
	lui	$1,0x3f80
	mtc1	$1,$f2
	nop
	nop
	div.s	$f1,$f1,$f3
	.word	0x46000834
	nop
	bc1t	loc_00151af4
	mov.s	$f12,$f0
	.word	0x46011034
	nop
	bc1f	loc_00151af4
	mov.s	$f12,$f1
	mov.s	$f12,$f2
loc_00151af4:
	jal	0x00263fb0
	nop
	lui	$1,0x56
	ld	$5,-0x79d0($1)
	jal	0x00262be8
	move	$4,$2
	lui	$1,0x56
	ld	$4,-0x79c8($1)
	jal	0x00262b80
	move	$5,$2
	jal	0x002633b8
	move	$4,$2
	lw	$4,0($29)
	mov.s	$f12,$f0
	jal	0x0014b330
	addiu	$5,$0,2
loc_00151b34:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00151a18
	nop
	nop
	addiu	$29,$29,-0xc0
	lui	$5,8
	sw	$4,0($29)
	sd	$22,0x90($29)
	lui	$4,0x56
	sd	$21,0x80($29)
	addiu	$4,$4,-0x79a8
	sd	$18,0x50($29)
	addiu	$21,$0,0x5a
	sd	$17,0x40($29)
	swc1	$f20,0xb0($29)
	sd	$31,0xa0($29)
	sd	$19,0x60($29)
	sd	$16,0x30($29)
	lw	$2,0($29)
	sd	$20,0x70($29)
	lw	$20,0x164($2)
	lui	$1,0x4396
	mtc1	$1,$f20
	ld	$2,0x20($20)
	addiu	$22,$20,0x178
	lw	$3,0x10($20)
	or	$2,$2,$5
	sd	$2,0x20($20)
	move	$5,$3
	jal	0x001a6e28
	sw	$3,0x440($20)
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x7990
	jal	0x00203aa0
	addiu	$4,$0,2
	lw	$18,0($29)
	addiu	$3,$0,1
	addiu	$4,$0,4
	lw	$2,0xc($18)
	xori	$2,$2,1
	jal	0x0013eb50
	movn	$4,$3,$2
	move	$17,$2
	beqz	$17,loc_00151ce8
	sw	$0,0x178($20)
	addiu	$19,$29,0x10
	lw	$2,0x16c($17)
loc_00151bf8:
	beqz	$2,loc_00151ccc
	move	$4,$18
	jal	0x0015eff8
	nop
	move	$16,$2
	jal	0x0015eff8
	move	$4,$17
	move	$4,$16
	jal	0x001943c8
	move	$5,$2
	.word	0x46140034
	nop
	bc1f	loc_00151ccc
	move	$4,$17
	jal	0x0015eff8
	nop
	move	$16,$2
	jal	0x0015eff8
	move	$4,$18
	move	$6,$2
	move	$5,$16
	jal	0x00243ae8
	addiu	$4,$29,0x20
	jal	0x0015ef88
	move	$4,$18
	move	$5,$2
	jal	0x00194508
	addiu	$4,$29,0x20
	bgez	$2,loc_00151c8c
	move	$4,$18
	jal	0x0015ef88
	nop
	move	$5,$2
	jal	0x00194508
	addiu	$4,$29,0x20
	b	loc_00151ca4
	negu	$3,$2
loc_00151c8c:
	jal	0x0015ef88
	move	$4,$18
	move	$5,$2
	jal	0x00194508
	addiu	$4,$29,0x20
	move	$3,$2
loc_00151ca4:
	slt	$2,$3,$21
	beqz	$2,loc_00151ccc
	lwc1	$f0,0x20($29)
	move	$21,$3
	lwc1	$f1,0x24($29)
	lwc1	$f2,0x28($29)
	sw	$17,0($22)
	swc1	$f0,0x10($29)
	swc1	$f1,0x14($29)
	swc1	$f2,0x18($29)
loc_00151ccc:
	jal	0x0013ebe0
	move	$4,$17
	move	$17,$2
	bnel	$17,$0,loc_00151bf8
	lw	$2,0x16c($17)
	b	loc_00151cf0
	lw	$2,0x178($20)
loc_00151ce8:
	addiu	$19,$29,0x10
	lw	$2,0x178($20)
loc_00151cf0:
	beqz	$2,loc_00151d4c
	nop
	lw	$4,0($29)
	jal	0x0014b048
	nop
	lui	$1,0x4120
	mtc1	$1,$f12
	lui	$1,0x42b4
	mtc1	$1,$f13
	beqz	$2,loc_00151d40
	move	$5,$19
	lw	$4,0($29)
	lui	$1,0x40a0
	mtc1	$1,$f12
	lui	$1,0x4234
	mtc1	$1,$f13
	jal	0x00106b80
	nop
	b	loc_00151d4c
	nop
loc_00151d40:
	lw	$4,0($29)
	jal	0x00106b80
	nop
loc_00151d4c:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	lw	$4,0($29)
	jal	0x00181bc0
	nop
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00151cf0
	lw	$2,0x178($20)
	nop
	.set	macro
	.set	reorder
	.end	actBoyWalk
