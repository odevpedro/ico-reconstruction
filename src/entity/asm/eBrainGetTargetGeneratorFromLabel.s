	.text
	.p2align 3
	.globl	eBrainGetTargetGeneratorFromLabel
	.ent	eBrainGetTargetGeneratorFromLabel
eBrainGetTargetGeneratorFromLabel:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x60
	sd	$16,0($29)
	move	$16,$4
	sd	$20,0x40($29)
	sd	$19,0x30($29)
	addiu	$19,$0,-1
	addiu	$20,$0,-1
	sd	$31,0x50($29)
	sd	$18,0x20($29)
	jal	0x00192b98
	sd	$17,0x10($29)
	.word	0x1453005c
	ld	$31,0x50($29)
	lui	$18,0x5f
	addiu	$17,$0,-1
	addiu	$3,$18,0x2fb8
	lw	$2,0x128($3)
	slt	$2,$16,$2
	bnez	$2,loc_00190dd8
	move	$6,$0
	lw	$2,0x12c($3)
	slt	$2,$16,$2
	beqz	$2,loc_00190ddc
	addiu	$6,$6,1
	.word	0x10000011
	move	$17,$0
loc_00190dd8:
	addiu	$6,$6,1
loc_00190ddc:
	slti	$2,$6,0x68
	.word	0x1040000d
	addiu	$2,$0,0x194
	addiu	$5,$18,0x2fb8
	.set	macro
	.set	reorder
	.end	eBrainGetTargetGeneratorFromLabel
