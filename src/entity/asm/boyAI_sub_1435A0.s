	.text
	.p2align 3
	.globl	boyAI_sub_1435A0
	.ent	boyAI_sub_1435A0
boyAI_sub_1435A0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0($29)
	beqz	$2,loc_00143618
	move	$18,$16
	addiu	$2,$0,0x194
	addiu	$3,$9,0x2fb8
	mult	$2,$10,$2
	addu	$4,$3,$2
	move	$3,$4
	lw	$4,0x10c($4)
	lw	$16,0x108($3)
	slt	$2,$16,$4
	beqz	$2,loc_00143618
	addiu	$2,$0,0x64
	lui	$3,0x5f
	mult	$2,$16,$2
	move	$6,$4
	addiu	$3,$3,-0x7090
	addiu	$7,$0,1
	addiu	$4,$2,0x60
	addu	$5,$2,$3
	addu	$4,$4,$3
	nop	
loc_001435f8:
	lw	$2,0($4)
	andi	$2,$2,1
	.word	0x1047ffcf
	addiu	$4,$4,0x64
	addiu	$16,$16,1
	slt	$2,$16,$6
	bnez	$2,loc_001435f8
	addiu	$5,$5,0x64
loc_00143618:
	.word	0x16600022
	lw	$8,-0x66e0($28)
	addiu	$2,$17,-0x6a50
	addiu	$7,$0,-1
	lhu	$3,2($2)
	addiu	$2,$0,0xb
	.word	0x14620007
	move	$4,$0
	lhu	$2,-0x6a50($17)
	.set	macro
	.set	reorder
	.end	boyAI_sub_1435A0
