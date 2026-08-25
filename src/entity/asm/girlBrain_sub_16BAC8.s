	.text
	.p2align 3
	.globl	girlBrain_sub_16BAC8
	.ent	girlBrain_sub_16BAC8
girlBrain_sub_16BAC8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bnez	$3,loc_0016baec
	lui	$2,0x27
	jal	0x00171ab0
	nop	
	beqz	$2,loc_0016bae8
	lw	$3,0($29)
	b	loc_0016bbd0
	addiu	$2,$0,9
loc_0016bae8:
	lui	$2,0x27
loc_0016baec:
	addiu	$5,$0,0xa
	lw	$4,0x4ec0($2)
	addiu	$3,$0,0x3c
	addiu	$2,$2,0x4ec0
	addiu	$7,$18,0x2ac0
	.word	0x00852018
	lw	$6,4($2)
	beql	$6,$0,loc_0016bb10
	break	0,7
loc_0016bb10:
	lw	$2,0x5900($7)
	subu	$3,$3,$4
	div	$0,$3,$6
	mflo	$3
	mult	$3,$3,$5
	slt	$2,$3,$2
	beqz	$2,loc_0016bb54
	addiu	$16,$18,0x2ac0
	beql	$6,$0,loc_0016bb38
	break	0,7
loc_0016bb38:
	lw	$2,0x58f8($7)
	slt	$2,$3,$2
	beqz	$2,loc_0016bb54
	lw	$2,0($29)
	b	loc_0016bbd4
	sw	$0,0($2)
	addiu	$16,$18,0x2ac0
loc_0016bb54:
	lbu	$2,0x58e0($16)
	beqz	$2,loc_0016bbd4
	nop	
	jal	0x0016b3c0
	move	$4,$0
	move	$3,$2
	bltz	$3,loc_0016bb78
	lw	$2,0($29)
	sw	$3,0($2)
loc_0016bb78:
	sb	$0,0x58e0($16)
	b	loc_0016bbd4
	addiu	$22,$0,1
	lw	$3,0($29)
	b	loc_0016bbd0
	addiu	$2,$0,6
	addiu	$3,$18,0x2ac0
	lbu	$2,0x58e0($3)
	beqz	$2,loc_0016bbd4
	lw	$2,0($29)
	b	loc_0016bbb4
	sw	$0,0($2)
	lw	$2,0($29)
	addiu	$3,$18,0x2ac0
	sw	$0,0($2)
loc_0016bbb4:
	b	loc_0016bbd4
	sb	$0,0x58e0($3)
	jal	0x00171ab0
	nop	
	bnez	$2,loc_0016bbd4
	lw	$3,0($29)
	addiu	$2,$0,3
loc_0016bbd0:
	sw	$2,0($3)
loc_0016bbd4:
	jal	0x0014a560
	nop	
	beqz	$2,loc_0016bc1c
	lw	$4,0($29)
	jal	0x0015eff8
	lw	$4,-0x6e0c($28)
	lw	$4,-0x6e08($28)
	jal	0x0015eff8
	move	$16,$2
	move	$4,$16
	jal	0x00194360
	move	$5,$2
	lwc1	$f1,-0x7b90($28)
	.word	0x46010034
	nop	
	bc1tl	loc_0016bc18
	addiu	$23,$0,1
loc_0016bc18:
	lw	$4,0($29)
loc_0016bc1c:
	addiu	$3,$0,2
	lw	$2,0($4)
	.word	0x5043000f
	lw	$3,0($4)
	jal	0x0014a560
	nop	
	.word	0x1040000a
	lw	$4,0($29)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16BAC8
