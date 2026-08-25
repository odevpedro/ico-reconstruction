	.text
	.p2align 3
	.globl	girlBrain_sub_16C170
	.ent	girlBrain_sub_16C170
girlBrain_sub_16C170:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x00a42818
	lw	$3,4($6)
	beql	$3,$0,loc_0016c180
	break	0,7
loc_0016c180:
	lw	$4,0x10($17)
	subu	$2,$2,$5
	div	$0,$2,$3
	mflo	$2
	div	$0,$4,$2
	mfhi	$5
	.word	0x14a00004
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16C170
