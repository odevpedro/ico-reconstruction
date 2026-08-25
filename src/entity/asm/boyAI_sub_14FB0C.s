	.text
	.p2align 3
	.globl	boyAI_sub_14FB0C
	.ent	boyAI_sub_14FB0C
boyAI_sub_14FB0C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$5,$0,0xce
	.word	0x1000019e
	nop	
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d48($28)
	.word	0x46010034
	.word	0x45000075
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xcd
	.word	0x10000194
	nop	
	lw	$2,0x2d0($18)
	andi	$2,$2,0x20
	.word	0x10400029
	addiu	$2,$0,1
	lw	$3,0x34($18)
	beq	$3,$2,loc_0014fb98
	lwc1	$f1,0x33c($18)
	lui	$2,0xffff
	ori	$2,$2,0xffff
	.word	0x10620017
	nop	
	lwc1	$f0,-0x7d44($28)
	.word	0x46010034
	.word	0x45000015
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,0x5a
	sltiu	$2,$2,0xb5
	.word	0x10400010
	nop	
	.word	0x10000131
	nop	
loc_0014fb98:
	lwc1	$f0,-0x7d40($28)
	.word	0x46010034
	.word	0x45000056
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,0x5a
	sltiu	$2,$2,0xb5
	.word	0x14400128
	nop	
	.word	0x1000004f
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_14FB0C
