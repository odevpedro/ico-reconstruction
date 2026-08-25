	.text
	.p2align 3
	.globl	boyAI_sub_14F408
	.ent	boyAI_sub_14F408
boyAI_sub_14F408:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d8c($28)
	.word	0x46010034
	bc1f	loc_0014f448
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,-0x2e
	sltiu	$2,$2,0x59
	beqz	$2,loc_0014f448
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x138
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d88($28)
	.word	0x46010034
loc_0014f448:
	bc1f	loc_0014f474
	nop	
	lw	$3,0x330($18)
	slti	$2,$3,-0x86
	bnez	$2,loc_0014f474
	slti	$2,$3,-0x2d
	beqz	$2,loc_0014f474
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x139
loc_0014f474:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x116
	.word	0x10000342
	nop	
	lw	$2,0x2d4($18)
	andi	$2,$2,0x40
	beql	$2,$0,loc_0014f4a8
	lwc1	$f1,0x33c($18)
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xd2
	lwc1	$f1,0x33c($18)
loc_0014f4a8:
	lwc1	$f0,-0x7d84($28)
	.word	0x46010034
	.word	0x4500000c
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,-0x2e
	sltiu	$2,$2,0x59
	.word	0x10400007
	.set	macro
	.set	reorder
	.end	boyAI_sub_14F408
