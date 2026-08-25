	.text
	.p2align 3
	.globl	iosOmExeEachGObj
	.ent	iosOmExeEachGObj
iosOmExeEachGObj:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x40
	lui	$2,0x28
	sd	$18,0x20($29)
	sll	$4,$4,2
	sd	$17,0x10($29)
	addiu	$2,$2,0x1a70
	sd	$31,0x30($29)
	addu	$4,$4,$2
	sd	$16,0($29)
	move	$18,$5
	lw	$16,0($4)
	beqz	$16,loc_0013fd5c
	move	$17,$6
	move	$4,$16
loc_0013fd48:
	jalr	$18
	move	$5,$17
	lw	$16,0x10($16)
	bnez	$16,loc_0013fd48
	move	$4,$16
loc_0013fd5c:
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x40
	.set	macro
	.set	reorder
	.end	iosOmExeEachGObj
