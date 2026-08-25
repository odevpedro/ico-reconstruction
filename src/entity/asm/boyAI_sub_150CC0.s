	.text
	.p2align 3
	.globl	boyAI_sub_150CC0
	.ent	boyAI_sub_150CC0
boyAI_sub_150CC0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	break	0,7
	lw	$5,0x48($20)
	subu	$2,$2,$4
	div	$0,$2,$3
	mflo	$2
	slt	$2,$2,$5
	beqz	$2,loc_00150d28
	ld	$2,0x468($20)
	dsrl32	$2,$2,0xe
	andi	$2,$2,1
	beqz	$2,loc_00150d04
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0xe
	andi	$2,$2,1
	bnez	$2,loc_00150d18
	nop	
loc_00150d04:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xe9
	b	loc_00150d28
	ld	$2,0x468($20)
loc_00150d18:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xea
	ld	$2,0x468($20)
loc_00150d28:
	dsrl32	$2,$2,0xd
	andi	$2,$2,1
	.word	0x1040000d
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0xd
	andi	$2,$2,1
	.word	0x10400008
	lw	$4,-0x6e08($28)
	.word	0x10800003
	.set	macro
	.set	reorder
	.end	boyAI_sub_150CC0
