	.text
	.p2align 3
	.globl	girlBrain_sub_16FD24
	.ent	girlBrain_sub_16FD24
girlBrain_sub_16FD24:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$3,0x27
	addiu	$5,$0,0xa
	lw	$2,0x4ec0($3)
	addiu	$6,$3,0x4ec0
	lw	$3,4($6)
	addiu	$4,$0,0x3c
	mult	$2,$2,$5
	beql	$3,$0,loc_0016fd48
	break	0,7
loc_0016fd48:
	subu	$4,$4,$2
	div	$0,$4,$3
	mflo	$3
	srl	$2,$3,0x1f
	addu	$2,$3,$2
	sra	$2,$2,1
	slt	$2,$16,$2
	.word	0x5440ffec
	sw	$0,0x33c($20)
	.word	0x10000011
	lui	$3,0x28
	nop	
	addiu	$2,$0,2
	addiu	$3,$29,0x10
	addiu	$4,$29,0x20
	sw	$2,0xc0($29)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16FD24
