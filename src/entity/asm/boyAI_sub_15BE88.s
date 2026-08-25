	.text
	.p2align 3
	.globl	boyAI_sub_15BE88
	.ent	boyAI_sub_15BE88
boyAI_sub_15BE88:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$4,$17
	jal	0x0015bcc8
	addiu	$5,$0,0x136
	lwc1	$f1,0x33c($16)
	lwc1	$f0,-0x7c68($28)
	.word	0x46010034
	bc1f	loc_0015bf20
	nop	
	lw	$2,0x330($16)
	addiu	$2,$2,0x86
	sltiu	$2,$2,0x10d
	bnez	$2,loc_0015bf20
	addiu	$2,$0,3
	lw	$7,0x30($16)
	bne	$7,$2,loc_0015bedc
	move	$4,$17
	ld	$2,0x18($16)
	dsrl32	$2,$2,7
	andi	$2,$2,1
	bnez	$2,loc_0015bf20
	nop	
loc_0015bedc:
	jal	0x0015bcc8
	addiu	$5,$0,0x137
	ld	$2,0x20($16)
	dsll	$2,$2,0x19
	dsra32	$2,$2,0
	andi	$2,$2,1
	beql	$2,$0,loc_0015bf20
	lwc1	$f1,0x33c($16)
	ld	$2,0x18($16)
	dsrl32	$2,$2,7
	andi	$2,$2,1
	bnel	$2,$0,loc_0015bf20
	lwc1	$f1,0x33c($16)
	move	$4,$17
	jal	0x0015bcc8
	addiu	$5,$0,0xaf
	lwc1	$f1,0x33c($16)
loc_0015bf20:
	lwc1	$f0,-0x7c64($28)
	.word	0x46010034
	bc1f	loc_0015bf58
	nop	
	lw	$2,0x330($16)
	addiu	$2,$2,-0x2e
	sltiu	$2,$2,0x59
	beqz	$2,loc_0015bf58
	move	$4,$17
	jal	0x0015bcc8
	addiu	$5,$0,0x138
	lwc1	$f1,0x33c($16)
	lwc1	$f0,-0x7c60($28)
	.word	0x46010034
loc_0015bf58:
	.word	0x4500000a
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_15BE88
