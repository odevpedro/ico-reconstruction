	.text
	.p2align 3
	.globl	queenbarrier_hB
	.ent	queenbarrier_hB
queenbarrier_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x1a0
	move	$5,$0
	sd	$21,0x150($29)
	addiu	$6,$0,0x10
	move	$21,$4
	sd	$22,0x160($29)
	sd	$19,0x130($29)
	move	$4,$29
	sd	$17,0x110($29)
	sd	$31,0x190($29)
	sd	$30,0x180($29)
	sd	$23,0x170($29)
	sd	$20,0x140($29)
	sd	$18,0x120($29)
	jal	0x002641d8
	sd	$16,0x100($29)
	lui	$1,0x44fa
	mtc1	$1,$f0
	addiu	$4,$0,0x2e
	lw	$2,0x15c($21)
	swc1	$f0,4($29)
	jal	0x0013eb50
	lw	$17,0x800($2)
	move	$19,$2
	lw	$4,-0x6f60($28)
	lw	$2,0x15c($19)
	addiu	$3,$0,0x25
	bne	$4,$3,loc_0019aaa0
	lw	$22,0x800($2)
	lui	$2,0x56
	b	loc_0019aaa8
	addiu	$18,$2,-0x3948
loc_0019aaa0:
	lui	$2,0x56
	addiu	$18,$2,-0x3940
loc_0019aaa8:
	lw	$2,-0x5c34($28)
	andi	$2,$2,1
	beqz	$2,loc_0019aad8
	move	$30,$0
	lui	$7,0x56
	lui	$6,0xffff
	lw	$8,0x18($17)
	addiu	$7,$7,-0x3938
	addiu	$4,$0,0xa
	addiu	$5,$0,0x46
	jal	0x001a6a30
	ori	$6,$6,0xffff
loc_0019aad8:
	move	$16,$0
	addiu	$20,$29,0x10
loc_0019aae0:
	bnez	$16,loc_0019ab10
	move	$23,$16
	jal	0x0013eae8
	lw	$4,0($18)
	beql	$2,$0,loc_0019aae0
	addiu	$16,$16,1
	lw	$2,0x15c($2)
	lw	$3,0x800($2)
	lb	$4,0x12($3)
	bnel	$4,$0,loc_0019aae0
	addiu	$16,$16,1
	move	$23,$16
loc_0019ab10:
	lw	$2,0($18)
	lw	$3,8($21)
	b	loc_0019ab28
	move	$16,$0
loc_0019ab20:
	bnez	$16,loc_0019ab38
	move	$4,$20
loc_0019ab28:
	bnel	$3,$2,loc_0019ab20
	addiu	$16,$16,1
	move	$30,$0
	move	$4,$20
loc_0019ab38:
	jal	0x00104508
	move	$5,$19
	lb	$2,0x12($17)
	beqz	$2,loc_0019ab80
	addiu	$16,$29,0x20
	lw	$2,0x16c($19)
	beqz	$2,loc_0019ab80
	nop	
	lw	$3,0($22)
	lui	$2,0xff00
	ori	$2,$2,0xff
	and	$3,$3,$2
	bnel	$3,$0,loc_0019ab84
	move	$5,$21
	lb	$2,1($22)
	bnez	$2,loc_0019abb0
	addiu	$16,$29,0xa0
	addiu	$16,$29,0x20
loc_0019ab80:
	move	$5,$21
loc_0019ab84:
	jal	0x00104140
	move	$4,$16
	addiu	$4,$29,0x50
	jal	0x00243b60
	move	$5,$29
	lw	$2,0x15c($21)
	move	$5,$16
	jal	0x00105f20
	lw	$4,0xc($2)
	b	loc_0019ad74
	lb	$2,0x10($17)
loc_0019abb0:
	lwc1	$f2,-0x7878($28)
	lwc1	$f1,-0x7874($28)
	negu	$4,$30
	lwc1	$f0,-0x7870($28)
	swc1	$f2,0xb0($29)
	swc1	$f1,0xb4($29)
	swc1	$f0,0xb8($29)
	sw	$0,0xbc($29)
	ld	$2,0xb0($29)
	ld	$3,0xb8($29)
	sd	$2,0xa0($29)
	bltz	$4,loc_0019abf4
	sd	$3,0xa8($29)
	mtc1	$4,$f1
	cvt.s.w	$f1,$f1
	b	loc_0019ac0c
	nop	
loc_0019abf4:
	srl	$3,$4,1
	andi	$2,$4,1
	or	$2,$2,$3
	mtc1	$2,$f1
	cvt.s.w	$f1,$f1
	add.s	$f1,$f1,$f1
loc_0019ac0c:
	lwc1	$f12,-0x786c($28)
	move	$4,$16
	lwc1	$f0,-0x7868($28)
	move	$5,$16
	mul.s	$f12,$f1,$f12
	jal	0x00243b18
	add.s	$f12,$f12,$f0
	addiu	$4,$17,0x20
	move	$6,$16
	jal	0x00243ad0
	move	$5,$4
	lwc1	$f13,-0x7864($28)
	jal	0x0025e700
	lwc1	$f12,0x20($17)
	lwc1	$f1,-0x7860($28)
	mov.s	$f2,$f0
	.word	0x46020834
	bc1f	loc_0019ac64
	nop	
	lwc1	$f0,-0x785c($28)
	b	loc_0019ac80
	sub.s	$f2,$f2,$f0
loc_0019ac64:
	lwc1	$f0,-0x7858($28)
	.word	0x46001034
	nop	
	bc1fl	loc_0019ac84
	lwc1	$f12,0x24($17)
	lwc1	$f0,-0x7854($28)
	add.s	$f2,$f2,$f0
loc_0019ac80:
	lwc1	$f12,0x24($17)
loc_0019ac84:
	lwc1	$f13,-0x7850($28)
	jal	0x0025e700
	swc1	$f2,0x20($17)
	lwc1	$f1,-0x784c($28)
	mov.s	$f2,$f0
	.word	0x46020834
	bc1f	loc_0019acb0
	nop	
	lwc1	$f0,-0x7848($28)
	b	loc_0019accc
	sub.s	$f2,$f2,$f0
loc_0019acb0:
	lwc1	$f0,-0x7844($28)
	.word	0x46001034
	nop	
	bc1fl	loc_0019acd0
	lwc1	$f12,0x28($17)
	lwc1	$f0,-0x7840($28)
	add.s	$f2,$f2,$f0
loc_0019accc:
	lwc1	$f12,0x28($17)
loc_0019acd0:
	lwc1	$f13,-0x783c($28)
	jal	0x0025e700
	swc1	$f2,0x24($17)
	lwc1	$f1,-0x7838($28)
	mov.s	$f2,$f0
	.word	0x46020834
	bc1f	loc_0019acfc
	nop	
	lwc1	$f0,-0x7834($28)
	b	loc_0019ad14
	sub.s	$f2,$f2,$f0
loc_0019acfc:
	lwc1	$f0,-0x7830($28)
	.word	0x46001034
	bc1f	loc_0019ad14
	nop	
	lwc1	$f0,-0x782c($28)
	add.s	$f2,$f2,$f0
loc_0019ad14:
	lui	$1,0x3f80
	mtc1	$1,$f0
	addiu	$16,$29,0x60
	swc1	$f2,0x28($17)
	addiu	$4,$29,0x90
	swc1	$f0,0x60($29)
	move	$5,$17
	sw	$0,0x64($29)
	sw	$0,0x68($29)
	sw	$0,0x6c($29)
	sw	$0,0x70($29)
	swc1	$f0,0x14($16)
	sw	$0,0x78($29)
	sw	$0,0x7c($29)
	sw	$0,0x80($29)
	sw	$0,0x84($29)
	swc1	$f0,0x28($16)
	jal	0x00243b60
	sw	$0,0x8c($29)
	lw	$2,0x15c($21)
	move	$5,$16
	jal	0x00105f20
	lw	$4,0xc($2)
	lb	$2,0x10($17)
loc_0019ad74:
	beqz	$2,loc_0019ae08
	addiu	$2,$23,-1
	bnel	$30,$2,loc_0019ae0c
	lw	$2,8($22)
	sw	$0,4($22)
	lw	$2,-0x6e0c($28)
	lw	$3,0x164($2)
	lw	$4,0x140($3)
	beqz	$4,loc_0019ada8
	addiu	$2,$0,0x12
	jal	0x001f41c8
	nop	
	addiu	$2,$0,0x12
loc_0019ada8:
	move	$4,$21
	sw	$2,8($22)
	jal	0x001d12a8
	addiu	$5,$0,0x5a
	lw	$2,0x18($17)
	addiu	$2,$2,1
	slti	$3,$2,5
	bnez	$3,loc_0019ae08
	sw	$2,0x18($17)
	addiu	$16,$29,0xc0
	move	$5,$21
	jal	0x00104140
	move	$4,$16
	addiu	$4,$29,0xf0
	jal	0x00243b60
	move	$5,$29
	lw	$2,0x15c($21)
	move	$5,$16
	jal	0x00105f20
	lw	$4,0xc($2)
	sb	$0,0x12($17)
	addiu	$5,$0,0x5d
	jal	0x001d12a8
	lw	$4,-0x6e0c($28)
loc_0019ae08:
	lw	$2,8($22)
loc_0019ae0c:
	blez	$2,loc_0019ae18
	addiu	$2,$2,-1
	sw	$2,8($22)
loc_0019ae18:
	jal	0x0019c798
	sb	$0,0x10($17)
	ld	$31,0x190($29)
	ld	$30,0x180($29)
	ld	$23,0x170($29)
	ld	$22,0x160($29)
	ld	$21,0x150($29)
	ld	$20,0x140($29)
	ld	$19,0x130($29)
	ld	$18,0x120($29)
	ld	$17,0x110($29)
	ld	$16,0x100($29)
	jr	$31
	addiu	$29,$29,0x1a0
	.set	macro
	.set	reorder
	.end	queenbarrier_hB
