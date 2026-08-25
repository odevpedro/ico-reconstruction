	.text
	.p2align 3
	.globl	boyAI_sub_14E710
	.ent	boyAI_sub_14E710
boyAI_sub_14E710:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$16,0x20($29)
	sd	$31,0x60($29)
	move	$16,$4
	sd	$19,0x50($29)
	sd	$18,0x40($29)
	beqz	$2,loc_0014e76c
	sd	$17,0x30($29)
	jal	0x0015ef88
	nop	
	lui	$1,0x42c8
	mtc1	$1,$f12
	move	$5,$2
	jal	0x00243b18
	move	$4,$29
	jal	0x0015eff8
	move	$4,$16
	move	$6,$2
	move	$4,$29
	jal	0x00243ad0
	move	$5,$29
	move	$4,$16
	jal	0x00104360
	move	$5,$29
loc_0014e76c:
	jal	0x0017b230
	addiu	$4,$0,0x15c
	beqz	$2,loc_0014e78c
	addiu	$17,$29,0x10
	jal	0x0017b288
	addiu	$4,$0,0x15c
	.word	0x10000028
	ld	$31,0x60($29)
loc_0014e78c:
	addiu	$18,$29,0x14
	addiu	$19,$29,0x18
	move	$4,$16
	move	$5,$0
	move	$6,$0
	move	$7,$17
	move	$8,$18
	jal	0x001b8120
	move	$9,$19
	.set	macro
	.set	reorder
	.end	boyAI_sub_14E710
