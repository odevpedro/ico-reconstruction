	.text
	.p2align 3
	.globl	boyAI_sub_14FBF8
	.ent	boyAI_sub_14FBF8
boyAI_sub_14FBF8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
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
	.word	0x4500ffea
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,0x5a
	sltiu	$2,$2,0xb5
	.word	0x14400108
	nop	
	.word	0x1000ffe3
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
	bc1f	loc_0014fcbc
	mov.s	$f2,$f1
	bc1f	loc_0014fca4
	nop	
	lwc1	$f0,-0x7d30($28)
	.word	0x46000834
	bc1t	loc_0014fcb8
	nop	
	lw	$2,0x2d0($18)
	andi	$2,$2,0x20
	bnez	$2,loc_0014fcb8
	nop	
loc_0014fca4:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xad
	.word	0x10000136
	nop	
loc_0014fcb8:
	lwc1	$f0,-0x7d2c($28)
loc_0014fcbc:
	.word	0x46020034
	.word	0x4500000e
	nop	
	lwc1	$f0,-0x7d28($28)
	.word	0x46000834
	.set	macro
	.set	reorder
	.end	boyAI_sub_14FBF8
