	.text
	.p2align 3
	.globl	boyAI_sub_163FF4
	.ent	boyAI_sub_163FF4
boyAI_sub_163FF4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$5,$29
	lwc1	$f1,-0x7bb4($28)
	.word	0x46010034
	nop	
	bc1f	loc_0016405c
	lwc1	$f0,0x14($29)
	lwc1	$f1,4($29)
	mtc1	$0,$f2
	sub.s	$f1,$f0,$f1
	.word	0x46020834
	bc1f	loc_00164048
	nop	
	neg.s	$f1,$f1
	lui	$1,0x4370
	mtc1	$1,$f0
	.word	0x46010034
	nop	
	bc1tl	loc_00164060
	lw	$3,0x164($17)
	b	loc_00164094
	nop	
loc_00164048:
	lui	$1,0x4370
	mtc1	$1,$f0
	.word	0x46010034
	bc1f	loc_00164094
	nop	
loc_0016405c:
	lw	$3,0x164($17)
loc_00164060:
	lw	$2,0x10($3)
	slti	$2,$2,0xc
	bnez	$2,loc_00164094
	nop	
	addiu	$4,$29,0x20
	jal	0x00104508
	move	$5,$17
	jal	0x001d04b8
	addiu	$4,$29,0x20
	beqz	$2,loc_00164094
	nop	
	jal	0x00163d60
	move	$4,$17
loc_00164094:
	beql	$30,$0,loc_001640d0
	lw	$2,0x30($19)
	jal	0x0015eff8
	move	$4,$17
	move	$16,$2
	jal	0x0015eff8
	move	$4,$20
	move	$4,$16
	jal	0x001943c8
	move	$5,$2
	mov.s	$f12,$f0
	move	$4,$17
	jalr	$30
	move	$5,$20
	lw	$2,0x30($19)
loc_001640d0:
	addiu	$18,$0,6
	.word	0x105200c8
	move	$4,$17
	move	$5,$21
	move	$6,$29
	jal	0x00201280
	move	$7,$0
	.word	0x54400018
	lwc1	$f0,0x3d0($19)
	.set	macro
	.set	reorder
	.end	boyAI_sub_163FF4
