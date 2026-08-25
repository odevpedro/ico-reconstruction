	.text
	.p2align 3
	.globl	boyAI_sub_14B0C0
	.ent	boyAI_sub_14B0C0
boyAI_sub_14B0C0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$18,0x20($29)
	swc1	$f20,0x40($29)
	move	$18,$4
	sd	$17,0x10($29)
	move	$4,$5
	sd	$16,0($29)
	move	$17,$0
	sd	$31,0x30($29)
	jal	0x0013eb50
	lwc1	$f20,0x226c($2)
	move	$16,$2
	beqz	$16,loc_0014b134
	move	$2,$17
	nop	
loc_0014b0f8:
	jal	0x0015eff8
	move	$4,$16
	move	$4,$2
	jal	0x00194398
	move	$5,$18
	.word	0x46140034
	bc1f	loc_0014b120
	nop	
	mov.s	$f20,$f0
	move	$17,$16
loc_0014b120:
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
	bnez	$16,loc_0014b0f8
	move	$2,$17
loc_0014b134:
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	lwc1	$f20,0x40($29)
	jr	$31
	addiu	$29,$29,0x50
	.set	macro
	.set	reorder
	.end	boyAI_sub_14B0C0
