/* 0x0013F9A0: core IOS/GObj init stub.
 * Initializes the shared tables, then hands off into the next block.
 */
	.text
	.globl	iosOmInit
	.ent	iosOmInit
iosOmInit:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x10
	sd	$31,0($29)
	jal	0x0013dda0
	addiu	$4,$0,0x140
	jal	0x0013f3e8
	addiu	$4,$0,0x500
	jal	0x0013f2c8
	nop	
	ld	$31,0($29)
	j	0x1fc2d0
	addiu	$29,$29,0x10
	.set	macro
	.set	reorder
	.end	iosOmInit
