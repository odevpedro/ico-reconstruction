	.text
	.p2align 3
	.globl	boyAI_sub_153058
	.ent	boyAI_sub_153058
boyAI_sub_153058:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$1,0x4396
	mtc1	$1,$f1
	lwc1	$f2,0x24($29)
	add.s	$f0,$f0,$f1
	.word	0x46020034
	nop	
	bc1fl	loc_00153078
	move	$16,$0
loc_00153078:
	beqz	$16,loc_00153090
	nop	
	lw	$2,0x2d0($18)
	andi	$2,$2,8
	beqz	$2,loc_00153098
	nop	
loc_00153090:
	beqz	$17,loc_001530ac
	nop	
loc_00153098:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x16c
	.word	0x10000097
	nop	
loc_001530ac:
	jal	0x001521f0
	nop	
	beqz	$2,loc_001530cc
	lw	$4,-0x6e08($28)
	beqz	$4,loc_001530cc
	lw	$6,-0x6714($28)
	jal	0x0013ff88
	addiu	$5,$0,0x16e
loc_001530cc:
	jal	0x0015f208
	lw	$4,-0x6e08($28)
	bnez	$2,loc_001530f0
	lw	$4,-0x6e08($28)
	beqz	$4,loc_001530f0
	lw	$6,-0x6714($28)
	jal	0x0013ff88
	addiu	$5,$0,0x16f
	lw	$4,-0x6e08($28)
loc_001530f0:
	lw	$2,0x164($4)
	lw	$3,0x30($2)
	.word	0x14730082
	nop	
	jal	0x0015eff8
	move	$17,$0
	lw	$4,-0x6e0c($28)
	jal	0x0015eff8
	move	$16,$2
	lwc1	$f1,4($16)
	lwc1	$f0,4($2)
	lw	$4,-0x6e0c($28)
	jal	0x0015eff8
	sub.s	$f20,$f1,$f0
	lw	$4,-0x6e08($28)
	jal	0x0015eff8
	move	$16,$2
	move	$4,$16
	jal	0x00194360
	move	$5,$2
	lwc1	$f1,-0x7cdc($28)
	.word	0x46010034
	.word	0x45000027
	nop	
	jal	0x0015eff8
	lw	$4,-0x6e0c($28)
	lw	$4,-0x6e08($28)
	jal	0x0015eff8
	move	$16,$2
	lwc1	$f0,4($16)
	lwc1	$f1,4($2)
	sub.s	$f0,$f0,$f1
	.word	0x46170034
	.word	0x45000010
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_153058
