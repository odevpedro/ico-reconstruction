	.text
	.p2align 3
	.globl	boyAI_sub_1596B0
	.ent	boyAI_sub_1596B0
boyAI_sub_1596B0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$2,$0,0x18
	beq	$3,$2,loc_00159714
	addiu	$2,$0,0x30
	.word	0x1062003f
	addiu	$18,$29,0x10
	.word	0x10000092
	nop	
	addiu	$2,$0,0x9a
	beq	$3,$2,loc_001596e8
	addiu	$2,$0,0x129
	.word	0x50620075
	ld	$3,0x18($17)
	.word	0x1000008b
	nop	
loc_001596e8:
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$3,0x15c($2)
	lw	$5,0($29)
	sw	$0,0x130($3)
	lw	$10,0x48($17)
	lw	$2,0x15c($4)
	sw	$0,0x134($2)
	lw	$3,0x15c($5)
	.word	0x100000ce
	sw	$0,0x138($3)
loc_00159714:
	lw	$3,0($29)
	addiu	$23,$0,1
	lw	$5,0($29)
	addiu	$30,$0,1
	lw	$2,0x15c($3)
	lw	$4,0($29)
	sw	$0,0x130($2)
	lw	$10,0x48($17)
	lw	$2,0x15c($5)
	sw	$0,0x134($2)
	.set	macro
	.set	reorder
	.end	boyAI_sub_1596B0
