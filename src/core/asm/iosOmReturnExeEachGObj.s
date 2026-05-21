	.text
	.p2align 3
	.globl	iosOmReturnExeEachGObj
	.ent	iosOmReturnExeEachGObj
iosOmReturnExeEachGObj:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x60
	lui	$2,0x28
	sd	$19,0x30($29)
	addiu	$2,$2,0x1a70
	sd	$18,0x20($29)
	sll	$4,$4,2
	sd	$17,0x10($29)
	addu	$4,$4,$2
	sd	$31,0x50($29)
	move	$19,$5
	sd	$20,0x40($29)
	move	$18,$6
	sd	$16,0($29)
	move	$17,$7
	lw	$16,0($4)
	beqz	$16,2f
	move	$2,$0
	addiu	$20,$0,1
	move	$4,$16
	nop	
0:
	jalr	$19
	move	$5,$18
	beqz	$17,1f
	nop	
	bne	$17,$20,1f
	nop	
	bnez	$2,3f
	ld	$31,0x50($29)
1:
	bnez	$16,0b
	move	$4,$16
2:
	ld	$31,0x50($29)
3:
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	iosOmReturnExeEachGObj
