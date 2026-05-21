	.text
	.p2align 3
	.globl	isysGObjProcAdd_Wrapper
	.ent	isysGObjProcAdd_Wrapper
isysGObjProcAdd_Wrapper:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$2,$7
	addiu	$sp,$sp,-0x10
	andi	$7,$6,0xff
	move	$9,$8
	move	$6,$5
	sd	$31,0x0($sp)
	move	$8,$2
	.word	0x0c04fcfc		# jal isysGObjProcAdd_
	move	$5,$4
	ld	$31,0x0($sp)
	jr	$31
	addiu	$sp,$sp,0x10
	.end	isysGObjProcAdd_Wrapper

	.globl	isysGObjProcAdd_NoCallback
	.ent	isysGObjProcAdd_NoCallback
isysGObjProcAdd_NoCallback:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$8,$7
	addiu	$sp,$sp,-0x10
	andi	$7,$6,0xff
	sd	$31,0x0($sp)
	move	$6,$5
	addiu	$9,$0,0x1800
	.word	0x0c04fcfc		# jal isysGObjProcAdd_
	move	$5,$0
	ld	$31,0x0($sp)
	jr	$31
	addiu	$sp,$sp,0x10
	.end	isysGObjProcAdd_NoCallback
