/* 0x0013F700: process-pool allocation helper.
 * Byte-exact reconstruction confirmed against SCUS_971.13.ELF.
 */
	.text
	.p2align 3
	.globl	isysGObjProcessAlloc
	.ent	isysGObjProcessAlloc
isysGObjProcessAlloc:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	addiu	$5,$0,0x94
	sd	$16,0($29)
	lui	$6,0x55
	move	$16,$4
	sd	$31,0x10($29)
	mult	$16,$5
	lw	$4,-0x68e8($28)
	addiu	$6,$6,0x7ad0
	mflo	$5
	jal	0x0013a0f8
	addiu	$7,$0,0x49
	move	$5,$2
	sw	$16,-0x4c44($28)
	sw	$5,-0x4c48($28)
	beqz	$16,1f
	move	$4,$0
	nop	
0:
	addiu	$2,$0,0x94
	mult	$3,$4,$2
	addiu	$4,$4,1
	addu	$2,$3,$5
	sltu	$3,$4,$16
	sw	$0,0($2)
	bnez	$3,0b
	nop	
1:
	ld	$31,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	isysGObjProcessAlloc
