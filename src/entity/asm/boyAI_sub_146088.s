	.text
	.p2align 3
	.globl	boyAI_sub_146088
	.ent	boyAI_sub_146088
boyAI_sub_146088:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x48($19)
	bnez	$2,loc_001460c4
	addiu	$18,$0,1
	lw	$4,0x670($4)
	addiu	$5,$0,2
	lw	$2,0x260($4)
	addiu	$2,$2,1
	sw	$2,0x260($4)
	lw	$3,0x164($17)
	lw	$4,0x670($3)
	lw	$2,0x260($4)
	slti	$2,$2,5
	b	loc_001460c4
	movz	$18,$5,$2
	sw	$0,0x260($2)
loc_001460c4:
	addiu	$2,$0,1
	beq	$18,$2,loc_001460ec
	slti	$2,$18,2
	.word	0x14400043
	ld	$31,0x60($29)
	addiu	$2,$0,2
	.word	0x1242002b
	ld	$20,0x50($29)
	.word	0x1000003f
	nop	
loc_001460ec:
	move	$4,$29
	move	$5,$0
	jal	0x002641d8
	addiu	$6,$0,0x10
	lw	$2,0($29)
	mtc1	$20,$f1
	cvt.s.w	$f1,$f1
	addiu	$4,$19,0x47c
	move	$5,$29
	ori	$2,$2,0x400
	addiu	$6,$0,0xf
	sw	$2,0($29)
	nop	
loc_00146120:
	lbu	$2,0($4)
	addiu	$6,$6,-1
	lbu	$3,0($5)
	addiu	$5,$5,1
	and	$2,$2,$3
	sb	$2,0($4)
	bgez	$6,loc_00146120
	addiu	$4,$4,1
	lui	$1,0x3e80
	.set	macro
	.set	reorder
	.end	boyAI_sub_146088
