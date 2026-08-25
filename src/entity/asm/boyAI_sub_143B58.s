	.text
	.p2align 3
	.globl	boyAI_sub_143B58
	.ent	boyAI_sub_143B58
boyAI_sub_143B58:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x001007a0
	move	$4,$0
	addiu	$2,$0,-1
	ld	$31,0x40($29)
	slt	$3,$2,$16
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	movn	$2,$0,$3
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x50
	nop	
	jr	$31
	lw	$2,-0x66dc($28)
	jr	$31
	lw	$2,-0x66e8($28)
	addiu	$29,$29,-0x60
	ld	$3,-0x4c18($28)
	sd	$20,0x40($29)
	sd	$19,0x30($29)
	move	$20,$5
	sd	$18,0x20($29)
	move	$19,$4
	sd	$17,0x10($29)
	addiu	$18,$0,1
	sd	$31,0x50($29)
	move	$17,$0
	sd	$16,0($29)
	dsllv	$5,$18,$17
	nop	
loc_00143bd0:
	and	$2,$3,$5
	.word	0x50400011
	ld	$2,0x18($19)
	addiu	$17,$17,1
	sltiu	$2,$17,0x40
	bnel	$2,$0,loc_00143bd0
	dsllv	$5,$18,$17
	lui	$16,0x55
	.set	macro
	.set	reorder
	.end	boyAI_sub_143B58
