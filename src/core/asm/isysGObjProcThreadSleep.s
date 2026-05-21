	.text
	.p2align 3
	.globl	isysGObjProcThreadSleep
	.ent	isysGObjProcThreadSleep
isysGObjProcThreadSleep:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	move	$16,$4
	beqz	$16,1f
	sd	$31,0x10($29)
	move	$4,$0
0:
	jal	0x0013d3d8
	addiu	$16,$16,-1
	bnez	$16,0b
	move	$4,$0
1:
	ld	$31,0x10($29)
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	isysGObjProcThreadSleep
