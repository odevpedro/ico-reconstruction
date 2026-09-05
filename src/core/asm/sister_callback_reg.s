.text
	.p2align 3
	.globl	sister_callback_reg
	.ent	sister_callback_reg
sister_callback_reg:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$8,$7
	addiu	$29,$29,-0x10
	andi	$7,$6,0xff
	sd	$31,0x0($29)
	move	$6,$5
	addiu	$9,$0,0x1800
	.word	0x0c04fcfc		# jal isysGObjProcAdd_
	move	$5,$4
	ld	$31,0x0($29)
	jr	$31
	addiu	$29,$29,0x10
	nop
	.end	sister_callback_reg