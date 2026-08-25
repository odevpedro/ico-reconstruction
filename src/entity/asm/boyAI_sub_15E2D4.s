	.text
	.p2align 3
	.globl	boyAI_sub_15E2D4
	.ent	boyAI_sub_15E2D4
boyAI_sub_15E2D4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$18,0xf0($29)
	sd	$17,0xe0($29)
	addiu	$18,$2,0x5060
	sd	$31,0x100($29)
	addiu	$17,$0,0x190
	sd	$16,0xd0($29)
	sw	$4,0($29)
	lui	$1,0x40a0
	mtc1	$1,$f20
loc_0015e2f8:
	lw	$16,0($29)
	addiu	$4,$29,0x10
	addiu	$6,$0,0x2c
	lw	$2,0x15c($16)
	lw	$3,0x4a0($2)
	mult	$2,$3,$17
	addu	$3,$2,$18
	lw	$2,0x188($3)
	andi	$2,$2,1
	beqz	$2,loc_0015e35c
	move	$5,$16
	jal	0x0014a100
	nop	
	addiu	$4,$29,0x20
	addiu	$6,$0,0x33
	jal	0x0014a100
	move	$5,$16
	lwc1	$f0,0x24($29)
	addiu	$4,$29,0x10
	sub.s	$f0,$f0,$f20
	jal	0x00168538
	swc1	$f0,0x24($29)
	lw	$2,0xa4($29)
	bnez	$2,loc_0015e360
	addiu	$3,$0,1
loc_0015e35c:
	move	$3,$0
loc_0015e360:
	beqz	$3,loc_0015e374
	addiu	$5,$0,0x9d
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
loc_0015e374:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015e2f8
	.set	macro
	.set	reorder
	.end	boyAI_sub_15E2D4
