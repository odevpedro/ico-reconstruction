	.text
	.p2align 3
	.globl	RequestClipCollision
	.ent	RequestClipCollision
RequestClipCollision:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	addiu	$5,$0,0x15
	sd	$16,0($29)
	move	$16,$4
	sd	$31,0x10($29)
	lui	$4,0x1c
	jal	0x00203b40
	addiu	$4,$4,0x34c0
	move	$3,$2
	sw	$0,0($16)
	sw	$16,0x20($3)
	ld	$31,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	nop
	.set	macro
	.set	reorder
	.end	RequestClipCollision
