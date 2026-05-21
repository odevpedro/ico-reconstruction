	.text
	.p2align 3
	.globl	isysGObjProcRemove
	.ent	isysGObjProcRemove
isysGObjProcRemove:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	sd	$31,0x10($29)
	jal	0x0013f638
	move	$16,$4
	lw	$2,0x10($16)
	bnez	$2,0f
	sw	$0,0($16)
	addiu	$4,$16,0x24
	ld	$31,0x10($29)
	ld	$16,0($29)
	j	0x13d3f8
	addiu	$29,$29,0x20
0:
	ld	$31,0x10($29)
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	isysGObjProcRemove
