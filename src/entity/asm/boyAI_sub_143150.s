	.text
	.p2align 3
	.globl	boyAI_sub_143150
	.ent	boyAI_sub_143150
boyAI_sub_143150:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sll	$3,$9,6
	addu	$16,$3,$6
	lh	$8,0x10($16)
	.word	0x05000033
	move	$11,$5
	lhu	$2,0($16)
	sra	$4,$4,8
	.word	0x14820030
	ld	$31,0x10($29)
	addiu	$2,$6,0x30
	addu	$10,$3,$2
	lw	$6,0($10)
	beqz	$6,loc_001431c0
	addiu	$3,$0,1
	ld	$7,0x20($6)
	dsllv	$3,$3,$9
	and	$2,$7,$3
	beqz	$2,loc_001431c0
	nor	$5,$0,$3
	lhu	$4,0($16)
	ld	$3,-0x4c10($28)
	and	$2,$7,$5
	sd	$2,0x20($6)
	addiu	$4,$4,1
	and	$3,$3,$5
	sh	$4,0($16)
	sd	$3,-0x4c10($28)
	sw	$0,0($10)
loc_001431c0:
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
	.word	0x14800007
	ld	$31,0x10($29)
loc_00143218:
	lw	$4,0xc($16)
	.word	0x10800004
	ld	$31,0x10($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_143150
