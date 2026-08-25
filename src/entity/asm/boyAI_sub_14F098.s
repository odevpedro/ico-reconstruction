	.text
	.p2align 3
	.globl	boyAI_sub_14F098
	.ent	boyAI_sub_14F098
boyAI_sub_14F098:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	blez	$2,loc_0014f0a8
	addiu	$2,$2,-1
	addiu	$7,$0,1
	sw	$2,-0x6648($28)
loc_0014f0a8:
	beqz	$7,loc_0014f0cc
	nop	
	lui	$1,0x3f00
	mtc1	$1,$f0
	.word	0x46010034
	bc1f	loc_0014f0cc
	nop	
	swc1	$f0,0x33c($18)
	mov.s	$f1,$f0
loc_0014f0cc:
	lwc1	$f0,-0x7db4($28)
	.word	0x46010034
	nop	
	bc1f	loc_0014f0ec
	sw	$19,0x160($29)
	sw	$0,0x160($29)
	.word	0x46010034
	nop	
loc_0014f0ec:
	bc1fl	loc_0014f128
	sw	$0,0x164($29)
	lwc1	$f0,-0x7db0($28)
	.word	0x46000834
	nop	
	bc1t	loc_0014f118
	lw	$2,0x164($29)
	lw	$2,0x2d0($18)
	andi	$2,$2,0x20
	beqz	$2,loc_0014f124
	lw	$2,0x164($29)
loc_0014f118:
	addiu	$2,$2,1
	b	loc_0014f128
	sw	$2,0x164($29)
loc_0014f124:
	sw	$0,0x164($29)
loc_0014f128:
	lwc1	$f0,-0x7dac($28)
	.word	0x46010034
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_14F098
