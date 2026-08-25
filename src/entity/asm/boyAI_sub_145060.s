	.text
	.p2align 3
	.globl	boyAI_sub_145060
	.ent	boyAI_sub_145060
boyAI_sub_145060:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$17,0x20($29)
	sd	$31,0x60($29)
	sd	$16,0x10($29)
	lw	$2,0x4b4($19)
	sll	$17,$2,2
	addu	$3,$19,$17
	lw	$2,0x320($3)
	beqz	$2,loc_001450bc
	move	$18,$4
	addu	$16,$17,$19
	move	$4,$29
	jal	0x00104508
	lw	$5,0($16)
	lw	$5,0($16)
	move	$4,$18
	lui	$1,0x4396
	mtc1	$1,$f12
	move	$6,$29
	jal	0x00144e58
	addiu	$7,$0,0x96
	addu	$3,$19,$17
	b	loc_001450c4
	sw	$2,0x190($3)
loc_001450bc:
	addu	$2,$19,$17
	sw	$0,0x190($2)
loc_001450c4:
	lw	$5,0x164($18)
	lw	$2,0x678($5)
	lw	$4,0x730($2)
	sltiu	$3,$4,8
	.word	0x10600081
	lui	$2,0x55
	sll	$3,$4,2
	addiu	$2,$2,0x7f10
	addu	$3,$3,$2
	lw	$4,0($3)
	jr	$4
	nop	
	jal	0x0015eff8
	move	$4,$18
	.set	macro
	.set	reorder
	.end	boyAI_sub_145060
