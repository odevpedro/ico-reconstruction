	.text
	.p2align 3
	.globl	subGirlBrain_Idle
	.ent	subGirlBrain_Idle
subGirlBrain_Idle:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	sd	$31,0x10($29)
	move	$4,$0
	lw	$2,0($29)
	lw	$3,0x164($2)
	jal	0x00203aa0
	sw	$0,0x33c($3)
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	.set	macro
	.set	reorder
	.end	subGirlBrain_Idle
