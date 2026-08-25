	.text
	.p2align 3
	.globl	boyAI_sub_14F104
	.ent	boyAI_sub_14F104
boyAI_sub_14F104:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x164($29)
	lw	$2,0x2d0($18)
	andi	$2,$2,0x20
	beqz	$2,loc_0014f124
	lw	$2,0x164($29)
	addiu	$2,$2,1
	b	loc_0014f128
	sw	$2,0x164($29)
loc_0014f124:
	sw	$0,0x164($29)
loc_0014f128:
	lwc1	$f0,-0x7dac($28)
	.word	0x46010034
	nop	
	bc1fl	loc_0014f174
	sw	$0,0x168($29)
	bc1f	loc_0014f16c
	lw	$3,0x168($29)
	lwc1	$f0,-0x7da8($28)
	.word	0x46000834
	nop	
	bc1tl	loc_0014f174
	sw	$0,0x168($29)
	lw	$2,0x2d0($18)
	andi	$2,$2,0x20
	bnel	$2,$0,loc_0014f174
	sw	$0,0x168($29)
	lw	$3,0x168($29)
loc_0014f16c:
	addiu	$3,$3,1
	sw	$3,0x168($29)
loc_0014f174:
	lw	$4,0($29)
	lw	$5,0x160($29)
	lw	$6,0x164($29)
	jal	0x0015bd48
	lw	$7,0x168($29)
	lw	$2,0x30($18)
	addiu	$2,$2,-1
	sltiu	$3,$2,0x72
	.word	0x106003fd
	lui	$4,0x56
	sll	$2,$2,2
	addiu	$3,$4,-0x7d00
	.set	macro
	.set	reorder
	.end	boyAI_sub_14F104
