	.text
	.p2align 3
	.globl	boyAI_sub_14F4A0
	.ent	boyAI_sub_14F4A0
boyAI_sub_14F4A0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$5,$0,0xd2
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d84($28)
	.word	0x46010034
	bc1f	loc_0014f4e4
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,-0x2e
	sltiu	$2,$2,0x59
	beqz	$2,loc_0014f4e4
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x138
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d80($28)
	.word	0x46010034
loc_0014f4e4:
	.word	0x45000205
	nop	
	lw	$3,0x330($18)
	slti	$2,$3,-0x86
	.word	0x14400201
	slti	$2,$3,-0x2d
	.word	0x104001ff
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x139
	.word	0x100001fa
	nop	
	lw	$3,0x2d4($18)
	andi	$2,$3,0x40
	beqz	$2,loc_0014f53c
	andi	$2,$3,0x10
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xd2
	b	loc_0014f554
	lwc1	$f1,0x33c($18)
loc_0014f53c:
	beql	$2,$0,loc_0014f554
	lwc1	$f1,0x33c($18)
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	lwc1	$f1,0x33c($18)
loc_0014f554:
	lwc1	$f0,-0x7d7c($28)
	.word	0x46010034
	bc1f	loc_0014f590
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,-0x2e
	sltiu	$2,$2,0x59
	beqz	$2,loc_0014f590
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x138
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d78($28)
	.word	0x46010034
loc_0014f590:
	.word	0x450001da
	nop	
	lw	$3,0x330($18)
	slti	$2,$3,-0x86
	.word	0x144001d6
	slti	$2,$3,-0x2d
	.word	0x104001d4
	nop	
	lw	$4,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14F4A0
