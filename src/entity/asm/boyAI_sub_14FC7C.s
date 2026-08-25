	.text
	.p2align 3
	.globl	boyAI_sub_14FC7C
	.ent	boyAI_sub_14FC7C
boyAI_sub_14FC7C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
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
	.word	0x46020034
	bc1f	loc_0014fcfc
	nop	
	lwc1	$f0,-0x7d28($28)
	.word	0x46000834
	bc1t	loc_0014fce8
	nop	
	lw	$2,0x2d0($18)
	andi	$2,$2,0x20
	beqz	$2,loc_0014fcfc
	nop	
loc_0014fce8:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xa8
	.word	0x10000125
	nop	
loc_0014fcfc:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x13a
	.word	0x10000120
	nop	
	jal	0x0014e840
	nop	
	move	$3,$2
	sltiu	$2,$3,5
	.word	0x1040001e
	sll	$2,$3,2
	lui	$4,0x56
	addiu	$3,$4,-0x7b30
	addu	$2,$2,$3
	lw	$4,0($2)
	jr	$4
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x98
	.word	0x10000013
	nop	
	lw	$4,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14FC7C
