	.text
	.p2align 3
	.globl	isysGObjAlloc
	.ent	isysGObjAlloc
isysGObjAlloc:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	addiu	$5,$0,0x174
	sd	$16,0($29)
	lui	$6,0x55
	move	$16,$4
	sd	$31,0x10($29)
	mult	$16,$5
	lw	$4,-0x68e8($28)
	addiu	$6,$6,0x7a10
	mflo	$5
	jal	0x0013a0f8
	addiu	$7,$0,0xae
	sw	$16,-0x4c4c($28)
	move	$4,$0
	beqz	$16,loc_0013e538
	sw	$2,-0x4c50($28)
	move	$3,$2
	addiu	$5,$0,-1
loc_0013e518:
	sw	$0,0($3)
	addiu	$4,$4,1
	sw	$0,0x15c($3)
	sltu	$2,$4,$16
	sw	$5,8($3)
	sw	$5,4($3)
	bnez	$2,loc_0013e518
	addiu	$3,$3,0x174
loc_0013e538:
	ld	$31,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	isysGObjAlloc
