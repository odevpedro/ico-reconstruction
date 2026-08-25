	.text
	.p2align 3
	.globl	boyAI_sub_15C994
	.ent	boyAI_sub_15C994
boyAI_sub_15C994:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	ld	$2,0x18($19)
	dsrl32	$2,$2,0x15
	andi	$2,$2,1
	beqz	$2,loc_0015c9c0
	nop	
	lw	$4,0($29)
	addiu	$5,$0,0xa7
	jal	0x0015bcc8
	sw	$0,4($29)
	.word	0x100000e9
	nop	
loc_0015c9c0:
	lw	$2,0($29)
	bne	$2,$7,loc_0015c9ec
	lw	$6,8($29)
	jal	0x0014a0d8
	nop	
	beqz	$2,loc_0015c9e8
	addiu	$8,$0,9
	addiu	$7,$0,0xa
	b	loc_0015c9f4
	lw	$6,0xc($29)
loc_0015c9e8:
	lw	$6,8($29)
loc_0015c9ec:
	move	$8,$0
	addiu	$7,$0,9
loc_0015c9f4:
	.word	0x1611008c
	move	$3,$8
	ld	$2,0x20($19)
	lw	$9,4($29)
	dsll	$2,$2,4
	dsra32	$2,$2,0
	addiu	$3,$9,1
	andi	$2,$2,1
	movz	$9,$3,$2
	.word	0x18c00048
	sw	$9,4($29)
	lui	$3,0x27
	addiu	$9,$0,0xa
	lw	$2,0x4ec0($3)
	move	$5,$3
	addiu	$5,$5,0x4ec0
	addiu	$4,$0,0x15e
	mult	$2,$2,$9
	lw	$3,4($5)
	addiu	$5,$6,1
	beql	$3,$0,loc_0015ca4c
	break	0,7
loc_0015ca4c:
	subu	$2,$30,$2
	div	$0,$2,$3
	mflo	$2
	.set	macro
	.set	reorder
	.end	boyAI_sub_15C994
