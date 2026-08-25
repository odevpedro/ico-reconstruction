	.text
	.p2align 3
	.globl	boyAI_sub_157B6C
	.ent	boyAI_sub_157B6C
boyAI_sub_157B6C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sdr	$2,0x350($3)
	sw	$4,0x358($3)
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$3,0x164($2)
	lw	$6,0($29)
	lw	$7,0x670($3)
	jal	0x00194ff8
	addiu	$7,$7,0x330
	lw	$5,0($29)
	jal	0x001572f0
	move	$4,$17
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ff8c
	nop	
	nop	
	lw	$3,0xc($5)
	slti	$2,$3,0x16
	bnez	$2,loc_00157bec
	move	$6,$4
	slti	$2,$3,0x18
	bnez	$2,loc_00157be0
	move	$4,$5
	slti	$2,$3,0x1a
	beqz	$2,loc_00157bec
	move	$5,$6
	j	0x1c0ae8
	addiu	$6,$0,0x16
loc_00157be0:
	move	$5,$6
	j	0x1c08b8
	addiu	$6,$0,0x16
loc_00157bec:
	jr	$31
	.set	macro
	.set	reorder
	.end	boyAI_sub_157B6C
