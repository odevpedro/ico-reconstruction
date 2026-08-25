	.text
	.p2align 3
	.globl	girlBrain_sub_16C2EC
	.ent	girlBrain_sub_16C2EC
girlBrain_sub_16C2EC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mtc1	$1,$f4
	div	$0,$4,$3
	lw	$2,0x678($5)
	lwc1	$f2,0x380($2)
	cvt.s.w	$f2,$f2
	mul.s	$f2,$f2,$f3
	mflo	$4
	mtc1	$4,$f1
	cvt.s.w	$f1,$f1
	mul.s	$f0,$f0,$f1
	nop	
	nop	
	div.s	$f12,$f2,$f0
	.word	0x460c2034
	nop	
	bc1f	loc_0016c344
	lw	$4,-0x6e0c($28)
	move	$5,$0
	jal	0x00182ab8
	addiu	$6,$0,1
	b	loc_0016c354
	nop	
loc_0016c344:
	lw	$2,0($29)
	lw	$3,0x164($2)
	lw	$4,0x678($3)
	sw	$0,0x380($4)
loc_0016c354:
	jal	0x0013eca8
	nop	
	move	$16,$2
	beql	$16,$0,loc_0016c3d0
	ld	$2,0x18($20)
	addiu	$19,$0,4
	addiu	$18,$0,1
	addiu	$17,$0,0x11
	lw	$3,0xc($16)
loc_0016c378:
	beq	$3,$19,loc_0016c3a0
	slti	$2,$3,5
	beqz	$2,loc_0016c398
	nop	
	beq	$3,$18,loc_0016c3a4
	lw	$4,0x84($29)
	b	loc_0016c3b8
	nop	
loc_0016c398:
	bne	$3,$17,loc_0016c3b8
	nop	
loc_0016c3a0:
	lw	$4,0x84($29)
loc_0016c3a4:
	jal	0x00104508
	move	$5,$16
	lw	$4,0($29)
	jal	0x0014af70
	move	$5,$16
loc_0016c3b8:
	jal	0x0013ecf8
	move	$4,$16
	move	$16,$2
	bnel	$16,$0,loc_0016c378
	lw	$3,0xc($16)
	ld	$2,0x18($20)
loc_0016c3d0:
	dsrl32	$2,$2,0xc
	andi	$2,$2,1
	beqz	$2,loc_0016c3fc
	lw	$4,0x74($29)
	jal	0x0016b790
	lw	$5,0x98($29)
	lw	$4,0x74($29)
	addiu	$3,$0,-1
	movn	$4,$3,$2
	b	loc_0016c400
	sw	$4,0x74($29)
loc_0016c3fc:
	sw	$0,0x70($29)
loc_0016c400:
	jal	0x0013eb50
	addiu	$4,$0,0x13
	move	$16,$2
	beqz	$16,loc_0016c470
	nop	
	lui	$17,0x6b
	addiu	$21,$0,8
	addiu	$19,$17,-0x4200
	addiu	$18,$0,1
	lw	$2,0x16c($16)
loc_0016c428:
	beqz	$2,loc_0016c45c
	nop	
	jal	0x001d3db0
	move	$4,$16
	beqz	$2,loc_0016c45c
	nop	
	jal	0x001f2148
	move	$4,$2
	beqz	$2,loc_0016c45c
	nop	
	sw	$21,0x70($29)
	sw	$16,4($19)
	sw	$18,-0x4200($17)
loc_0016c45c:
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
	bnel	$16,$0,loc_0016c428
	lw	$2,0x16c($16)
loc_0016c470:
	lw	$2,0($29)
	lw	$3,0x164($2)
	lw	$4,0x678($3)
	lw	$16,0x3c0($4)
	.word	0x12000051
	lw	$8,-0x6e0c($28)
	jal	0x0015eff8
	move	$4,$16
	lwc1	$f0,0($2)
	move	$4,$16
	jal	0x0015eff8
	swc1	$f0,0x20($29)
	lwc1	$f0,4($2)
	move	$4,$16
	jal	0x0015eff8
	swc1	$f0,0x24($29)
	lwc1	$f0,8($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x28($29)
	lwc1	$f0,0($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x30($29)
	lwc1	$f0,4($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x34($29)
	lwc1	$f0,8($2)
	move	$4,$30
	lw	$5,0x88($29)
	jal	0x00194360
	swc1	$f0,0x38($29)
	lwc1	$f1,-0x7b8c($28)
	.word	0x46010034
	nop	
	.word	0x45000030
	lwc1	$f3,0x24($29)
	lwc1	$f2,0x34($29)
	mtc1	$0,$f0
	sub.s	$f1,$f3,$f2
	.word	0x46000834
	.word	0x4500000a
	nop	
	neg.s	$f1,$f1
	lui	$1,0x447a
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	.word	0x4503000a
	sub.s	$f1,$f3,$f2
	.word	0x10000022
	lw	$8,-0x6e0c($28)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16C2EC
