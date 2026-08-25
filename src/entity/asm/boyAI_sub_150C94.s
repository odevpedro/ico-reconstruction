	.text
	.p2align 3
	.globl	boyAI_sub_150C94
	.ent	boyAI_sub_150C94
boyAI_sub_150C94:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x10830023
	addiu	$2,$0,0x5c
	.word	0x10820021
	lui	$3,0x27
	addiu	$5,$0,0xa
	lw	$4,0x4ec0($3)
	addiu	$6,$3,0x4ec0
	lw	$3,4($6)
	addiu	$2,$0,0x3c
	.word	0x00852018
	beql	$3,$0,loc_00150cc4
	break	0,7
loc_00150cc4:
	lw	$5,0x48($20)
	subu	$2,$2,$4
	div	$0,$2,$3
	mflo	$2
	slt	$2,$2,$5
	.word	0x10400013
	ld	$2,0x468($20)
	dsrl32	$2,$2,0xe
	andi	$2,$2,1
	beqz	$2,loc_00150d04
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0xe
	andi	$2,$2,1
	.word	0x14400006
	nop	
loc_00150d04:
	lw	$4,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_150C94
