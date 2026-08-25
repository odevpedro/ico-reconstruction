	.text
	.p2align 3
	.globl	boyAI_sub_14E7AC
	.ent	boyAI_sub_14E7AC
boyAI_sub_14E7AC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$9,$19
	beqz	$2,loc_0014e824
	lbu	$2,-0x4c00($28)
	bnez	$2,loc_0014e828
	ld	$31,0x60($29)
	lbu	$2,-0x4c01($28)
	bnel	$2,$0,loc_0014e82c
	ld	$19,0x50($29)
	jal	0x00203aa0
	addiu	$4,$0,3
	move	$7,$17
	move	$8,$18
	move	$9,$19
	move	$4,$16
	move	$5,$0
	jal	0x001b8120
	move	$6,$0
	jal	0x00203aa0
	lw	$4,0x10($29)
	jal	0x0017b528
	move	$4,$16
	move	$4,$16
	jal	0x0017e240
	addiu	$5,$0,7
	jal	0x00203aa0
	lw	$4,0x14($29)
	jal	0x0017b568
	move	$4,$16
	jal	0x00203aa0
	lw	$4,0x18($29)
loc_0014e824:
	ld	$31,0x60($29)
loc_0014e828:
	ld	$19,0x50($29)
loc_0014e82c:
	ld	$18,0x40($29)
	ld	$17,0x30($29)
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0x70
	addiu	$29,$29,-0x70
	sd	$18,0x30($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14E7AC
