	.text
	.p2align 3
	.globl	girlBrain_sub_16E0A4
	.ent	girlBrain_sub_16E0A4
girlBrain_sub_16E0A4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	add.s	$f20,$f21,$f20
	.word	0x4615a034
	nop	
	bc1t	loc_0016e0bc
	lw	$16,0x94($29)
	.word	0x46140834
loc_0016e0bc:
	ld	$2,0x18($16)
	dsrl32	$2,$2,0x15
	andi	$2,$2,1
	.word	0x5040001d
	lw	$3,0x340($16)
	.word	0x16c0001b
	lw	$3,0x340($16)
	lw	$4,0x30($29)
	lw	$2,-0x6e0c($28)
	.word	0x14820017
	nop	
	addiu	$2,$0,7
	.word	0x14620015
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E0A4
