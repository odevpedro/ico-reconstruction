	.text
	.p2align 3
	.globl	boyAI_sub_14B48C
	.ent	boyAI_sub_14B48C
boyAI_sub_14B48C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$5,$0,-1
	mult	$3,$3,$2
	addu	$3,$3,$4
	lh	$3,0x182($3)
	beq	$3,$5,loc_0014b4c0
	move	$2,$0
	mtc1	$3,$f1
	cvt.s.w	$f1,$f1
	lwc1	$f0,0x4ac($6)
	.word	0x46010034
	nop	
	bc1tl	loc_0014b4c0
	addiu	$2,$0,1
loc_0014b4c0:
	jr	$31
	nop	
	addiu	$29,$29,-0x40
	addiu	$2,$0,0x182
	sd	$18,0x20($29)
	sd	$17,0x10($29)
	move	$18,$8
	sd	$16,0($29)
	move	$17,$7
	sd	$31,0x30($29)
	beq	$5,$2,loc_0014b504
	move	$16,$4
	addiu	$2,$0,0x183
	.word	0x10a20016
	ld	$31,0x30($29)
	.word	0x1000001b
	ld	$18,0x20($29)
loc_0014b504:
	beqz	$18,loc_0014b520
	move	$4,$16
	lw	$2,0x164($16)
	lh	$3,0x12a($2)
	.word	0x1c600014
	ld	$31,0x30($29)
	move	$4,$16
loc_0014b520:
	jal	0x0013ff88
	addiu	$5,$0,0x182
	.word	0x1220000e
	addiu	$4,$0,-2
	lw	$3,0x164($16)
	andi	$5,$18,1
	ld	$2,0x128($3)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14B48C
