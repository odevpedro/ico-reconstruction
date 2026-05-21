	.text
	.p2align 3
	.globl	iosOmGetGObjStatus
	.ent	iosOmGetGObjStatus
iosOmGetGObjStatus:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	addiu	$2,$0,0x140
	sd	$16,0($29)
	move	$16,$5
	sd	$31,0x10($29)
	jal	0x0013e488
	sw	$2,0($4)
	sw	$2,0($16)
	ld	$31,0x10($29)
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	iosOmGetGObjStatus
