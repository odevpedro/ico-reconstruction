	.text
	.p2align 3
	.globl	girlBrain_sub_16C540
	.ent	girlBrain_sub_16C540
girlBrain_sub_16C540:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$8,-0x6e0c($28)
	lui	$1,0x447a
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_0016c5c8
	lw	$8,-0x6e0c($28)
	sub.s	$f1,$f3,$f2
	mtc1	$0,$f0
	.word	0x46000834
	bc1f	loc_0016c594
	nop	
	neg.s	$f1,$f1
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46010034
	nop	
	bc1t	loc_0016c5ac
	lui	$5,0x6b
	b	loc_0016c5c8
	lw	$8,-0x6e0c($28)
loc_0016c594:
	lui	$1,0x42c8
	mtc1	$1,$f0
	.word	0x46010034
	nop	
	bc1f	loc_0016c5c4
	lui	$5,0x6b
loc_0016c5ac:
	addiu	$2,$0,8
	addiu	$3,$5,-0x4200
	addiu	$4,$0,2
	sw	$16,4($3)
	sw	$2,0x70($29)
	sw	$4,-0x4200($5)
loc_0016c5c4:
	lw	$8,-0x6e0c($28)
loc_0016c5c8:
	move	$16,$0
	beqz	$8,loc_0016c780
	move	$17,$0
	lw	$4,0x164($8)
	addiu	$3,$0,0x31
	lw	$2,0x30($4)
	bnel	$2,$3,loc_0016c78c
	addiu	$17,$0,1
	lw	$4,0x148($4)
	beql	$4,$0,loc_0016c78c
	addiu	$17,$0,1
	jal	0x001c0690
	nop	
	addiu	$3,$0,7
	bne	$2,$3,loc_0016c60c
	move	$2,$0
	addiu	$2,$0,1
loc_0016c60c:
	andi	$2,$2,0xff
	lui	$1,0x4316
	mtc1	$1,$f20
	beqz	$2,loc_0016c628
	nop	
	lui	$1,0x4396
	mtc1	$1,$f20
loc_0016c628:
	jal	0x0015ef88
	lw	$4,-0x6e0c($28)
	lwc1	$f0,0($2)
	lw	$4,-0x6e0c($28)
	jal	0x0015ef88
	swc1	$f0,0x60($29)
	lwc1	$f0,4($2)
	lw	$4,-0x6e0c($28)
	jal	0x0015ef88
	swc1	$f0,0x64($29)
	lw	$3,-0x6e0c($28)
	lwc1	$f0,8($2)
	lw	$2,0x164($3)
	swc1	$f0,0x68($29)
	jal	0x0015eff8
	lw	$4,0x148($2)
	lw	$3,-0x6e0c($28)
	lwc1	$f0,0($2)
	lw	$2,0x164($3)
	swc1	$f0,0x30($29)
	jal	0x0015eff8
	lw	$4,0x148($2)
	lw	$3,-0x6e0c($28)
	lwc1	$f0,4($2)
	lw	$2,0x164($3)
	swc1	$f0,0x34($29)
	jal	0x0015eff8
	lw	$4,0x148($2)
	lwc1	$f0,8($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x38($29)
	lwc1	$f0,0($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x20($29)
	lwc1	$f0,4($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x24($29)
	lwc1	$f0,8($2)
	lw	$4,0x90($29)
	lui	$1,0x42c8
	mtc1	$1,$f12
	lw	$5,0x94($29)
	jal	0x00243b18
	swc1	$f0,0x28($29)
	lw	$4,0x8c($29)
	lw	$5,0x88($29)
	jal	0x00243ad0
	lw	$6,0x90($29)
	lw	$4,0x8c($29)
	jal	0x00194360
	move	$5,$30
	mul.s	$f1,$f20,$f20
	.word	0x46010034
	nop	
	bc1tl	loc_0016c714
	addiu	$16,$0,1
loc_0016c714:
	lui	$1,0xc2c8
	mtc1	$1,$f12
	lw	$4,0x90($29)
	jal	0x00243b18
	lw	$5,0x94($29)
	lw	$4,0x8c($29)
	lw	$5,0x88($29)
	jal	0x00243ad0
	lw	$6,0x90($29)
	lw	$4,0x8c($29)
	jal	0x00194360
	move	$5,$30
	lwc1	$f1,-0x7b88($28)
	.word	0x46010034
	nop	
	bc1tl	loc_0016c758
	addiu	$16,$0,1
loc_0016c758:
	lw	$4,0x88($29)
	jal	0x00194360
	move	$5,$30
	lwc1	$f1,-0x7b84($28)
	.word	0x46010034
	nop	
	bc1fl	loc_0016c788
	move	$16,$0
	b	loc_0016c78c
	lw	$8,-0x6e0c($28)
loc_0016c780:
	b	loc_0016c78c
	addiu	$17,$0,1
loc_0016c788:
	lw	$8,-0x6e0c($28)
loc_0016c78c:
	beqz	$17,loc_0016c79c
	lui	$6,0x6b
	addiu	$2,$6,-0x4200
	sw	$0,0xc($2)
loc_0016c79c:
	beqz	$16,loc_0016c7ec
	lw	$5,0x4ec0($22)
	addiu	$2,$0,0xa
	addiu	$6,$22,0x4ec0
	addiu	$3,$0,0x3c
	.word	0x00a22818
	lw	$4,4($6)
	lw	$2,0x164($8)
	lui	$9,0x6b
	addiu	$7,$9,-0x4200
	lw	$6,0x148($2)
	subu	$3,$3,$5
	div	$0,$3,$4
	sw	$6,8($7)
	beql	$4,$0,loc_0016c7dc
	break	0,7
loc_0016c7dc:
	mflo	$3
	sll	$2,$3,2
	addu	$2,$2,$3
	sw	$2,0xc($7)
loc_0016c7ec:
	lui	$2,0x6b
	addiu	$6,$2,-0x4200
	lw	$3,0xc($6)
	beqz	$3,loc_0016c820
	addiu	$3,$3,-1
	lw	$2,8($6)
	sw	$3,0xc($6)
	addiu	$4,$0,8
	addiu	$5,$0,3
	sw	$2,4($6)
	lui	$3,0x6b
	sw	$4,0x70($29)
	sw	$5,-0x4200($3)
loc_0016c820:
	beqz	$8,loc_0016c86c
	addiu	$4,$0,0x34
	lw	$2,0x164($8)
	lw	$3,0x30($2)
	bne	$3,$4,loc_0016c870
	lw	$5,0x70($29)
	jal	0x0015f248
	move	$4,$8
	beqz	$2,loc_0016c870
	lw	$5,0x70($29)
	lw	$4,0($29)
	jal	0x0015f248
	nop	
	beqz	$2,loc_0016c86c
	addiu	$3,$0,0x4000
	sw	$0,0x70($29)
	ld	$2,0x20($20)
	or	$2,$2,$3
	sd	$2,0x20($20)
loc_0016c86c:
	lw	$5,0x70($29)
loc_0016c870:
	lw	$4,0x74($29)
	beq	$5,$4,loc_0016c900
	sll	$3,$5,2
	addiu	$16,$23,0x2ac0
	lui	$5,0x28
	lw	$4,-0x6714($28)
	addiu	$2,$5,0x28c0
	move	$6,$20
	addu	$3,$3,$2
	sw	$0,0x58f8($16)
	jal	0x00201de0
	lw	$5,0($3)
	lw	$6,0x70($29)
	addiu	$2,$0,1
	sw	$6,0x74($29)
	sb	$2,0x58e2($16)
	lw	$5,0x74($29)
	sltiu	$2,$5,0xa
	beqz	$2,loc_0016cc18
	sw	$6,0x57dc($16)
	lui	$7,0x56
	sll	$2,$5,2
	addiu	$3,$7,-0x6a60
	addu	$2,$2,$3
	lw	$4,0($2)
	jr	$4
	nop	
	b	loc_0016c8fc
	sw	$0,0x434($20)
	b	loc_0016c8f8
	addiu	$2,$0,1
	b	loc_0016c8f8
	addiu	$2,$0,2
	addiu	$2,$0,3
loc_0016c8f8:
	sw	$2,0x434($20)
loc_0016c8fc:
	lw	$5,0x70($29)
loc_0016c900:
	sltiu	$2,$5,0xa
	beqz	$2,loc_0016cc18
	lui	$8,0x56
	sll	$2,$5,2
	addiu	$3,$8,-0x6a30
	addu	$2,$2,$3
	lw	$4,0($2)
	jr	$4
	nop	
	lui	$9,0x29
	addiu	$2,$9,-0x5770
	b	loc_0016cc18
	sw	$0,0x18($2)
	ld	$3,0x18($20)
	ori	$2,$0,0xc000
	dsll32	$2,$2,8
	and	$2,$3,$2
	beqz	$2,loc_0016c95c
	nop	
	ori	$2,$0,0x8000
	dsll32	$2,$2,0xa
	or	$2,$3,$2
	sd	$2,0x18($20)
loc_0016c95c:
	lw	$4,0($29)
	addiu	$5,$0,0x13f
	jal	0x0015bcc8
	addiu	$16,$23,0x2ac0
	lw	$3,0x4ec0($22)
	addiu	$2,$0,0xa
	addiu	$5,$22,0x4ec0
	addiu	$6,$0,0x3c
	mult	$3,$3,$2
	lw	$4,4($5)
	lui	$5,0x29
	lui	$1,0x4040
	mtc1	$1,$f0
	addiu	$2,$5,-0x5770
	addiu	$7,$0,0x3c
	lw	$5,0x58f0($16)
	subu	$3,$6,$3
	swc1	$f0,0x18($2)
	div	$0,$3,$4
	beql	$4,$0,loc_0016c9b0
	break	0,7
loc_0016c9b0:
	lw	$2,0x58f4($16)
	move	$4,$5
	addiu	$5,$5,1
	mflo	$3
	mult	$2,$2,$3
	div	$0,$2,$6
	mflo	$2
	slt	$2,$2,$4
	beqz	$2,loc_0016c9e8
	sw	$5,0x58f0($16)
	lw	$4,0x58e8($16)
	move	$2,$29
	jal	0x0016bd08
	sltiu	$4,$4,1
loc_0016c9e8:
	lw	$5,0x58ec($16)
	lw	$4,0($29)
	jal	0x0014b228
	addiu	$5,$5,0xb
	lw	$4,0($29)
	mtc1	$0,$f12
	b	loc_0016cb64
	addiu	$5,$0,8
	lw	$4,0($29)
	addiu	$5,$0,0x13f
	jal	0x0015bcc8
	addiu	$16,$23,0x2ac0
	lw	$3,0x4ec0($22)
	addiu	$2,$0,0xa
	addiu	$5,$22,0x4ec0
	addiu	$6,$0,0x3c
	mult	$3,$3,$2
	lw	$4,4($5)
	lui	$5,0x29
	lui	$1,0x40c0
	mtc1	$1,$f0
	addiu	$2,$5,-0x5770
	addiu	$7,$0,0x3c
	lw	$5,0x58f0($16)
	subu	$3,$6,$3
	swc1	$f0,0x18($2)
	div	$0,$3,$4
	beql	$4,$0,loc_0016ca5c
	break	0,7
loc_0016ca5c:
	lw	$2,0x58f4($16)
	move	$4,$5
	addiu	$5,$5,1
	mflo	$3
	mult	$2,$2,$3
	div	$0,$2,$6
	mflo	$2
	slt	$2,$2,$4
	beqz	$2,loc_0016ca94
	sw	$5,0x58f0($16)
	lw	$4,0x58e8($16)
	move	$2,$29
	jal	0x0016bd08
	sltiu	$4,$4,1
loc_0016ca94:
	lw	$5,0x58ec($16)
	lw	$4,0($29)
	jal	0x0014b228
	addiu	$5,$5,5
	lwc1	$f12,0x58ec($16)
	cvt.s.w	$f12,$f12
	lw	$4,0($29)
	b	loc_0016cb64
	addiu	$5,$0,5
	lui	$1,0x4040
	mtc1	$1,$f0
	lui	$3,0x29
	addiu	$2,$3,-0x5770
	lw	$4,0($29)
	addiu	$5,$0,0x13f
	jal	0x0015bcc8
	swc1	$f0,0x18($2)
	addiu	$16,$23,0x2ac0
	lw	$3,0x4ec0($22)
	addiu	$2,$0,0xa
	addiu	$6,$22,0x4ec0
	addiu	$7,$0,0x3c
	mult	$3,$3,$2
	lw	$5,4($6)
	lw	$4,0x58f0($16)
	addiu	$6,$0,0x3c
	beql	$5,$0,loc_0016cb04
	break	0,7
loc_0016cb04:
	lw	$2,0x58f4($16)
	move	$8,$4
	subu	$3,$7,$3
	div	$0,$3,$5
	addiu	$4,$4,1
	mflo	$3
	mult	$2,$2,$3
	div	$0,$2,$7
	mflo	$2
	slt	$2,$2,$8
	beqz	$2,loc_0016cb44
	sw	$4,0x58f0($16)
	lw	$4,0x58e8($16)
	move	$2,$29
	jal	0x0016bd08
	sltiu	$4,$4,1
loc_0016cb44:
	lw	$5,0x58ec($16)
	lw	$4,0($29)
	jal	0x0014b228
	addiu	$5,$5,8
	lwc1	$f12,0x58ec($16)
	cvt.s.w	$f12,$f12
	lw	$4,0($29)
	addiu	$5,$0,8
loc_0016cb64:
	jal	0x0014b270
	move	$6,$0
	lw	$4,0($29)
	jal	0x0016ac20
	nop	
	b	loc_0016cc1c
	lui	$2,0x29
	lw	$6,0x4ec0($22)
	addiu	$4,$0,0xa
	addiu	$5,$22,0x4ec0
	addiu	$3,$0,0x3c
	.word	0x00c43018
	lw	$2,4($5)
	lui	$5,0x29
	lui	$1,0x4040
	mtc1	$1,$f0
	addiu	$4,$5,-0x5770
	addiu	$7,$23,0x2ac0
	swc1	$f0,0x18($4)
	move	$5,$0
	subu	$3,$3,$6
	beql	$2,$0,loc_0016cbc0
	break	0,7
loc_0016cbc0:
	div	$0,$3,$2
	lw	$4,0x58f8($7)
	mflo	$3
	srl	$2,$3,0x1f
	addu	$3,$3,$2
	sra	$3,$3,1
	div	$0,$4,$3
	mflo	$4
	andi	$4,$4,1
	beql	$4,$0,loc_0016cbf4
	lw	$2,0xc90($7)
	b	loc_0016cbfc
	lw	$5,-0x6e0c($28)
loc_0016cbf4:
	bnel	$2,$0,loc_0016cbfc
	lw	$5,0xca0($7)
loc_0016cbfc:
	beqz	$5,loc_0016cc1c
	lui	$2,0x29
	lw	$4,0($29)
	move	$6,$0
	addiu	$7,$0,2
	jal	0x0014b168
	addiu	$8,$0,1
loc_0016cc18:
	lui	$2,0x29
loc_0016cc1c:
	addiu	$4,$23,0x2ac0
	addiu	$6,$2,-0x5770
	addiu	$5,$0,-1
	lw	$2,0x58f8($4)
	lh	$3,0x24($6)
	addiu	$2,$2,1
	mtc1	$0,$f22
	beq	$3,$5,loc_0016cc44
	sw	$2,0x58f8($4)
	lwc1	$f22,0x20($6)
loc_0016cc44:
	jal	0x0014b358
	addiu	$4,$0,0x17
	mov.s	$f21,$f0
	jal	0x0014b358
	addiu	$4,$0,0x18
	mov.s	$f20,$f0
	jal	0x0014b358
	addiu	$4,$0,0x17
	sub.s	$f20,$f20,$f0
	addiu	$4,$0,0x17
	mul.s	$f20,$f22,$f20
	jal	0x0014b358
	add.s	$f22,$f21,$f20
	.word	0x4600b034
	nop	
	bc1t	loc_0016cca0
	addiu	$4,$0,0x17
	jal	0x0014b358
	addiu	$4,$0,0x18
	.word	0x46160034
	nop	
	bc1f	loc_0016ccb0
	addiu	$4,$0,0x18
loc_0016cca0:
	jal	0x0014b358
	nop	
	b	loc_0016ccb4
	mov.s	$f20,$f0
loc_0016ccb0:
	mov.s	$f20,$f22
loc_0016ccb4:
	lui	$3,0x29
	lui	$1,0x3f80
	mtc1	$1,$f1
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16C540
