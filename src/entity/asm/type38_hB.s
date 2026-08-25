	.text
	.p2align 3
	.globl	type38_hB
	.ent	type38_hB
type38_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0xd0
	lw	$3,-0x5178($28)
	sd	$31,0xb0($29)
	sd	$30,0xa0($29)
	sd	$23,0x90($29)
	sd	$22,0x80($29)
	sd	$21,0x70($29)
	sd	$20,0x60($29)
	sd	$19,0x50($29)
	sd	$18,0x40($29)
	sd	$17,0x30($29)
	sd	$16,0x20($29)
	swc1	$f21,0xc8($29)
	swc1	$f20,0xc0($29)
	sw	$4,0($29)
	lw	$2,0x15c($4)
	lw	$2,0x800($2)
	beqz	$3,loc_001f6808
	sw	$2,4($29)
	lw	$19,0($2)
	lw	$5,4($19)
	blez	$5,loc_001f67ec
	move	$17,$0
	lw	$6,0($19)
	addiu	$3,$0,0x50
	nop	
loc_001f6758:
	addiu	$2,$0,0x1a0
	.word	0x02232018
	mult	$2,$17,$2
	move	$18,$0
	addu	$3,$4,$6
	lw	$4,8($19)
	lw	$21,0($3)
	addu	$2,$2,$4
	lw	$23,4($2)
	blez	$21,loc_001f67d0
	lw	$22,0($2)
	lui	$30,0x27
	addiu	$20,$17,1
	nop	
loc_001f6790:
	addiu	$5,$0,0x50
	sll	$16,$18,4
	mult	$2,$17,$5
	addu	$4,$22,$16
	addiu	$18,$18,1
	addu	$5,$2,$6
	jal	0x00105f00
	addiu	$5,$5,0x20
	addu	$4,$23,$16
	jal	0x00105f00
	addiu	$5,$30,0x5850
	slt	$2,$18,$21
	bnez	$2,loc_001f6790
	lw	$6,0($19)
	b	loc_001f67d4
	lw	$5,4($19)
loc_001f67d0:
	addiu	$20,$17,1
loc_001f67d4:
	move	$17,$20
	slt	$2,$17,$5
	bnez	$2,loc_001f6758
	addiu	$3,$0,0x50
	b	loc_001f67f4
	lw	$4,0($29)
loc_001f67ec:
	lw	$6,0($19)
	lw	$4,0($29)
loc_001f67f4:
	jal	0x001f60d8
	addiu	$5,$6,0x20
	lw	$4,4($29)
	sw	$0,-0x5178($28)
	sw	$0,0xc($4)
loc_001f6808:
	jal	0x001f5700
	lw	$4,0($29)
	lw	$2,4($29)
	lui	$1,0x3f80
	mtc1	$1,$f2
	lwc1	$f1,0xc($2)
	.word	0x46020834
	nop	
	bc1f	loc_001f6978
	lw	$4,0($29)
	lwc1	$f0,-0x74cc($28)
	add.s	$f0,$f1,$f0
	.word	0x46001034
	nop	
	bc1f	loc_001f684c
	swc1	$f0,0xc($2)
	swc1	$f2,0xc($2)
loc_001f684c:
	lw	$4,0($29)
	move	$18,$0
	lwc1	$f1,-0x74c8($28)
	lw	$3,0x15c($4)
	lw	$2,0x800($3)
	swc1	$f2,8($2)
	lw	$2,4($29)
	lw	$3,0x15c($4)
	lwc1	$f0,0xc($2)
	lw	$8,0x800($3)
	lw	$21,0($8)
	lw	$4,4($21)
	blez	$4,loc_001f6974
	mul.s	$f20,$f0,$f1
	mov.s	$f21,$f2
	addiu	$2,$0,0x50
	nop	
loc_001f6890:
	addiu	$19,$0,1
	mult	$3,$18,$2
	lw	$2,0($21)
	addu	$3,$3,$2
	lw	$20,0($3)
	slt	$2,$19,$20
	beqz	$2,loc_001f6960
	sll	$23,$18,2
	lw	$4,8($21)
	addiu	$30,$20,-1
	addiu	$22,$18,1
	nop	
loc_001f68c0:
	mtc1	$19,$f12
	cvt.s.w	$f12,$f12
	addiu	$2,$0,0x1a0
	mtc1	$30,$f0
	cvt.s.w	$f0,$f0
	.word	0x02428818
	lw	$2,4($8)
	sll	$16,$19,4
	mul.s	$f12,$f20,$f12
	addiu	$19,$19,1
	addu	$2,$23,$2
	lw	$7,0($2)
	addu	$3,$17,$4
	nop	
	nop	
	div.s	$f12,$f12,$f0
	lw	$4,0($3)
	.word	0x7fa80010
	addu	$4,$4,$16
	cvt.w.s	$f0,$f12
	mfc1	$6,$f0
	mtc1	$6,$f0
	cvt.s.w	$f0,$f0
	sll	$6,$6,4
	addiu	$5,$6,0x10
	sub.s	$f12,$f12,$f0
	addu	$6,$7,$6
	jal	0x00244418
	addu	$5,$7,$5
	lw	$2,8($21)
	slt	$3,$19,$20
	move	$4,$2
	addu	$17,$17,$4
	lw	$2,0($17)
	addu	$16,$16,$2
	swc1	$f21,0xc($16)
	bnez	$3,loc_001f68c0
	.word	0x7ba80010
	b	loc_001f6964
	lw	$4,4($21)
loc_001f6960:
	addiu	$22,$18,1
loc_001f6964:
	move	$18,$22
	slt	$2,$18,$4
	bnez	$2,loc_001f6890
	addiu	$2,$0,0x50
loc_001f6974:
	lw	$4,0($29)
loc_001f6978:
	ld	$31,0xb0($29)
	ld	$30,0xa0($29)
	ld	$23,0x90($29)
	ld	$22,0x80($29)
	ld	$21,0x70($29)
	ld	$20,0x60($29)
	ld	$19,0x50($29)
	ld	$18,0x40($29)
	ld	$17,0x30($29)
	ld	$16,0x20($29)
	lwc1	$f21,0xc8($29)
	lwc1	$f20,0xc0($29)
	j	0x1f5b58
	addiu	$29,$29,0xd0
	.set	macro
	.set	reorder
	.end	type38_hB
