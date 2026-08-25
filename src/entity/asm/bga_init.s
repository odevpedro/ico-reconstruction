	.text
	.p2align 3
	.globl	bga_init
	.ent	bga_init
bga_init:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	sd	$31,0x10($29)
	lw	$2,0($29)
	lw	$4,0($29)
	jal	0x00202208
	nop	
	jal	0x00203aa0
	addiu	$4,$0,1
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	bga_init
