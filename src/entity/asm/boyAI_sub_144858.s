	.text
	.p2align 3
	.globl	boyAI_sub_144858
	.ent	boyAI_sub_144858
boyAI_sub_144858:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x001a6e28
	addiu	$4,$4,0x7eb0
loc_00144860:
	jal	0x0013d9c8
	move	$4,$0
	jal	0x0013d3f0
	nop	
loc_00144870:
	jal	0x0025c570
	nop	
	bnez	$2,loc_00144870
	nop	
	jal	0x001181e0
	nop	
	jal	0x0025c638
	nop	
	lw	$2,-0x60c0($28)
	bnez	$2,loc_001448a4
	nop	
	jal	0x001444a0
	nop	
loc_001448a4:
	jal	0x00143b88
	nop	
	beq	$16,$2,loc_001448c4
	nop	
	jal	0x00143b88
	nop	
	jal	0x00140f90
	move	$16,$2
loc_001448c4:
	jal	0x00118268
	nop	
	b	loc_00144860
	nop	
	nop	
	move	$6,$4
	lw	$5,0x164($6)
	ld	$2,0x18($5)
	dsrl32	$2,$2,3
	andi	$2,$2,1
	.word	0x1040000c
	addiu	$2,$0,0x1e
	lw	$4,0x48($5)
	beql	$2,$0,loc_00144900
	break	0,7
loc_00144900:
	div	$0,$4,$2
	mfhi	$3
	.word	0x14600005
	move	$4,$6
	lw	$5,0x434($5)
	lw	$7,-0x6f60($28)
	j	0x1ae420
	.set	macro
	.set	reorder
	.end	boyAI_sub_144858
