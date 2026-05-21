	.text
	.p2align 3
	.globl	isysGObjProcRemoveAll
	.ent	isysGObjProcRemoveAll
isysGObjProcRemoveAll:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	sd	$31,0x10($29)
	sd	$16,0($29)
	lw	$16,0x2c($4)
	beqz	$16,1f
	ld	$31,0x10($29)
0:
	jal	0x0013f6b8
	move	$4,$16
	lw	$16,8($16)
	bnez	$16,0b
	ld	$31,0x10($29)
1:
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	isysGObjProcRemoveAll
