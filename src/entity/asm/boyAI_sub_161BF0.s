	.text
	.p2align 3
	.globl	boyAI_sub_161BF0
	.ent	boyAI_sub_161BF0
boyAI_sub_161BF0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$2,0x150($6)
	jal	0x00190f30
	move	$4,$7
	move	$3,$2
	bnel	$3,$0,loc_00161c14
	lw	$2,4($3)
	sw	$0,0($17)
	b	loc_00161c1c
	move	$3,$0
loc_00161c14:
	lhu	$3,0($3)
	sw	$2,0($17)
loc_00161c1c:
	sw	$3,0($16)
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	nop	
	addiu	$29,$29,-0x30
	move	$6,$4
	move	$9,$5
	addiu	$3,$6,3
	move	$5,$2
	sw	$2,0($29)
	sd	$31,0x20($29)
	sltiu	$2,$3,0xc
	.word	0x1040009a
	sd	$16,0x10($29)
	lui	$2,0x56
	sll	$3,$3,2
	addiu	$2,$2,-0x7000
	addu	$3,$3,$2
	lw	$4,0($3)
	jr	$4
	nop	
	lw	$8,0($5)
	addiu	$6,$0,0x1c
	lui	$4,0x28
	lw	$3,0x164($8)
	addiu	$4,$4,0x2400
	lw	$5,0x20($4)
	lw	$7,0x670($3)
	lw	$2,0x200($7)
	mult	$2,$2,$6
	addu	$4,$4,$2
	lw	$3,4($4)
	.set	macro
	.set	reorder
	.end	boyAI_sub_161BF0
