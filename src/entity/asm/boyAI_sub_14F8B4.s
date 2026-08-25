	.text
	.p2align 3
	.globl	boyAI_sub_14F8B4
	.ent	boyAI_sub_14F8B4
boyAI_sub_14F8B4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x46000834
	nop	
	bc1fl	loc_0014f8c8
	move	$19,$0
	move	$17,$0
loc_0014f8c8:
	lw	$2,0x2d0($18)
	andi	$2,$2,0x20
	beqz	$2,loc_0014f960
	nop	
	beqz	$17,loc_0014f91c
	nop	
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d58($28)
	.word	0x46010034
	bc1f	loc_0014f91c
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,0x5a
	sltiu	$2,$2,0xb5
	beqz	$2,loc_0014f91c
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x79
	.word	0x1000021d
	nop	
loc_0014f91c:
	.word	0x126000f7
	nop	
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d54($28)
	.word	0x46010034
	.word	0x450000f2
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,0x59
	sltiu	$2,$2,0xb3
	.word	0x144000ed
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x78
	.word	0x1000020c
	nop	
loc_0014f960:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x13a
	.word	0x1000016a
	nop	
	lui	$3,0x27
	lui	$4,0x27
	lw	$2,0x4ec0($3)
	addiu	$5,$4,0x4ec0
	addiu	$3,$0,0xa
	addiu	$6,$0,0x3c
	mult	$2,$2,$3
	lw	$4,4($5)
	addiu	$7,$0,0x5a
	addiu	$3,$0,0x3c
	beql	$4,$0,loc_0014f9a4
	break	0,7
loc_0014f9a4:
	subu	$2,$6,$2
	lw	$3,0x48($18)
	div	$0,$2,$4
	mflo	$2
	mult	$2,$2,$7
	div	$0,$2,$6
	mflo	$2
	slt	$3,$3,$2
	.word	0x10600006
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	.set	macro
	.set	reorder
	.end	boyAI_sub_14F8B4
