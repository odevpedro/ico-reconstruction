	.text
	.p2align 3
	.globl	boyAI_sub_1431C0
	.ent	boyAI_sub_1431C0
boyAI_sub_1431C0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bnez	$11,loc_001431d8
	nop	
	jal	0x0025d7f0
	move	$4,$8
	b	loc_001431e4
	lw	$4,0x38($16)
loc_001431d8:
	jal	0x0025d7f0
	ori	$4,$8,0x8000
	lw	$4,0x38($16)
loc_001431e4:
	addiu	$3,$0,1
	lw	$2,0x38($4)
	srl	$2,$2,6
	andi	$2,$2,1
	beq	$2,$3,loc_00143218
	lui	$3,0x5f
	lhu	$2,0x36($4)
	addiu	$3,$3,0x2f00
	sll	$2,$2,3
	addu	$2,$2,$3
	lh	$4,6($2)
	bnez	$4,loc_00143230
	ld	$31,0x10($29)
loc_00143218:
	lw	$4,0xc($16)
	beqz	$4,loc_00143230
	ld	$31,0x10($29)
	jal	0x0013b8f0
	nop	
	ld	$31,0x10($29)
loc_00143230:
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	j	0x143138
	move	$5,$0
	j	0x143138
	addiu	$5,$0,1
	lui	$3,0x6b
	andi	$2,$4,0xff
	addiu	$3,$3,-0x6750
	sll	$2,$2,6
	addu	$2,$2,$3
	lh	$3,0x10($2)
	bltz	$3,loc_00143284
	sra	$4,$4,8
	lhu	$2,0($2)
	bne	$4,$2,loc_00143284
	nop	
	j	0x25da68
	move	$4,$3
loc_00143284:
	jr	$31
	nop	
	nop	
	lw	$2,-0x6f60($28)
	addiu	$4,$0,0x194
	addiu	$29,$29,-0x60
	mult	$2,$2,$4
	sd	$19,0x40($29)
	lui	$19,0x5f
	sd	$31,0x50($29)
	addiu	$3,$19,0x2fb8
	.set	macro
	.set	reorder
	.end	boyAI_sub_1431C0
