	.text
	.p2align 3
	.globl	girlBrain_sub_16E4A8
	.ent	girlBrain_sub_16E4A8
girlBrain_sub_16E4A8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lbu	$2,0x80($29)
	beqz	$2,loc_0016e4c8
	nop	
	lw	$4,0($29)
	jal	0x0016d330
	lw	$5,0xf8($29)
	lw	$16,0x94($29)
	movn	$20,$0,$2
loc_0016e4c8:
	.word	0x1280ffbe
	addiu	$2,$0,4
	sw	$0,0x90($29)
	.word	0x10000031
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16E4A8
