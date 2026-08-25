	.text
	.p2align 3
	.globl	boyAI_sub_1447AC
	.ent	boyAI_sub_1447AC
boyAI_sub_1447AC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$4,$0,2
	jal	0x00144308
	addiu	$4,$0,1
	addiu	$4,$0,1
	jal	0x00141f58
	move	$5,$0
	move	$4,$16
	move	$5,$17
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x1434b0
	addiu	$29,$29,0x30
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	sd	$31,0x10($29)
	jal	0x00144500
	move	$16,$4
	jal	0x00140b70
	move	$4,$0
	addiu	$3,$0,0x194
	lui	$2,0x5f
	.word	0x02038018
	addiu	$2,$2,0x2fb8
	ld	$31,0x10($29)
	addu	$16,$16,$2
	lhu	$4,0x18c($16)
	ld	$16,0($29)
	j	0x1412c8
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0x20
	lui	$4,0x55
	sd	$16,0($29)
	addiu	$4,$4,0x7e88
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$16,$0,-1
	lw	$5,-0x6e30($28)
	lui	$4,0x55
	jal	0x001a6e28
	addiu	$4,$4,0x7ea0
	lui	$4,0x55
	jal	0x001a6e28
	addiu	$4,$4,0x7eb0
	jal	0x0013d9c8
	move	$4,$0
	jal	0x0013d3f0
	nop	
loc_00144870:
	jal	0x0025c570
	nop	
	bnez	$2,loc_00144870
	nop	
	jal	0x001181e0
	nop	
	jal	0x0025c638
	nop	
	lw	$2,-0x60c0($28)
	bnez	$2,loc_001448a4
	nop	
	jal	0x001444a0
	nop	
loc_001448a4:
	jal	0x00143b88
	nop	
	.word	0x12020005
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_1447AC
