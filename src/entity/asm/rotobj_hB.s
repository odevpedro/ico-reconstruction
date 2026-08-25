	.text
	.p2align 3
	.globl	rotobj_hB
	.ent	rotobj_hB
rotobj_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x15c($4)
	lw	$5,0x800($2)
	lw	$3,0x30($5)
	move	$2,$3
	addiu	$3,$3,1
	slti	$2,$2,0x1f
	bnez	$2,loc_001e9978
	sw	$3,0x30($5)
	j	0x1ae460
	sw	$0,0x30($5)
loc_001e9978:
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	rotobj_hB
