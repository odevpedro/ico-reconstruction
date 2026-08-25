	.text
	.p2align 3
	.globl	girlBrain_sub_16E6A4
	.ent	girlBrain_sub_16E6A4
girlBrain_sub_16E6A4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sll	$2,$3,4
	subu	$2,$2,$3
	lw	$3,0xf0($29)
	slt	$2,$2,$3
	.word	0x10400091
	addiu	$2,$0,2
	sw	$0,0x90($29)
	.word	0x1000008e
	sw	$2,0x340($7)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E6A4
