	.text
	.p2align 3
	.globl	boyAI_sub_150CC8
	.ent	boyAI_sub_150CC8
boyAI_sub_150CC8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
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
	beqz	$2,loc_00150d68
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0xd
	andi	$2,$2,1
	beqz	$2,loc_00150d68
	lw	$4,-0x6e08($28)
	beqz	$4,loc_00150d5c
	lw	$6,-0x6714($28)
	jal	0x0013ff88
	addiu	$5,$0,0x3b
loc_00150d5c:
	lw	$4,0($29)
	.word	0x10000028
	addiu	$5,$0,0xea
loc_00150d68:
	jal	0x00175c18
	nop	
	.word	0x54400019
	ld	$2,0x468($20)
	lw	$7,-0x6e08($28)
	addiu	$3,$0,0x55
	lw	$2,0x164($7)
	lw	$4,0x38($2)
	.word	0x10830021
	addiu	$2,$0,0x5c
	.set	macro
	.set	reorder
	.end	boyAI_sub_150CC8
