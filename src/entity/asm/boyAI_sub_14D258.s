	.text
	.p2align 3
	.globl	boyAI_sub_14D258
	.ent	boyAI_sub_14D258
boyAI_sub_14D258:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x00149d60
	move	$10,$16
	move	$10,$16
	move	$4,$18
	addiu	$5,$0,1
	addiu	$6,$0,2
	addiu	$7,$0,1
	move	$8,$17
	jal	0x00149d60
	move	$9,$0
	b	loc_0014d2c8
	nop	
	move	$4,$18
	move	$5,$0
	addiu	$6,$0,2
	move	$7,$0
	move	$8,$0
	move	$9,$0
	jal	0x00149d60
	move	$10,$0
	move	$4,$18
	addiu	$5,$0,1
	addiu	$6,$0,2
	move	$7,$0
	move	$8,$0
	move	$9,$0
	jal	0x00149d60
	move	$10,$0
loc_0014d2c8:
	bnez	$22,loc_0014d2d8
	lui	$2,0x56
	.word	0x13c00012
	nop	
loc_0014d2d8:
	lw	$5,0x15c($18)
	addiu	$4,$2,0x5060
	addiu	$3,$0,0x190
	lw	$2,0x4a0($5)
	mult	$2,$2,$3
	addu	$2,$2,$4
	lw	$3,0x18c($2)
	srl	$3,$3,2
	andi	$3,$3,1
	.set	macro
	.set	reorder
	.end	boyAI_sub_14D258
