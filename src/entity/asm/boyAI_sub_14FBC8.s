	.text
	.p2align 3
	.globl	boyAI_sub_14FBC8
	.ent	boyAI_sub_14FBC8
boyAI_sub_14FBC8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x46010034
loc_0014fbcc:
	.word	0x4500004b
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,0x59
	sltiu	$2,$2,0xb3
	.word	0x10400129
	nop	
	.word	0x10000044
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x13a
	.word	0x100000c6
	nop	
	lw	$2,0x2d0($18)
	andi	$2,$2,0x20
	.word	0x104000c2
	nop	
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d38($28)
	.word	0x46010034
	bc1f	loc_0014fbcc
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,0x5a
	sltiu	$2,$2,0xb5
	.word	0x14400108
	nop	
	b	loc_0014fbcc
	nop	
	lw	$2,0x2d4($18)
	andi	$2,$2,0x20
	beql	$2,$0,loc_0014fc68
	lwc1	$f1,0x33c($18)
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb8
	.word	0x1000014a
	nop	
loc_0014fc68:
	lwc1	$f0,-0x7d34($28)
	.word	0x46010034
	nop	
	.word	0x45000011
	mov.s	$f2,$f1
	.word	0x45000009
	.set	macro
	.set	reorder
	.end	boyAI_sub_14FBC8
