	.text
	.p2align 3
	.globl	boyAI_sub_1448C4
	.ent	boyAI_sub_1448C4
boyAI_sub_1448C4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x00118268
	nop	
	.word	0x1000ffe4
	nop	
	nop	
	move	$6,$4
	lw	$5,0x164($6)
	ld	$2,0x18($5)
	dsrl32	$2,$2,3
	andi	$2,$2,1
	beqz	$2,loc_00144920
	addiu	$2,$0,0x1e
	lw	$4,0x48($5)
	beql	$2,$0,loc_00144900
	break	0,7
loc_00144900:
	div	$0,$4,$2
	mfhi	$3
	bnez	$3,loc_00144920
	move	$4,$6
	lw	$5,0x434($5)
	lw	$7,-0x6f60($28)
	j	0x1ae420
	move	$6,$0
loc_00144920:
	jr	$31
	nop	
	lw	$5,8($4)
	j	0x1ae6f8
	lw	$4,0xc($4)
	nop	
	addiu	$29,$29,-0x50
	move	$2,$4
	sd	$17,0x10($29)
	addiu	$17,$0,0x28
	sd	$16,0($29)
	.word	0x00518818
	lui	$16,0x56
	addiu	$16,$16,0x1928
	sw	$2,-0x6f40($28)
	sd	$19,0x30($29)
	sd	$18,0x20($29)
	move	$19,$5
	addu	$8,$16,$17
	sd	$31,0x40($29)
	move	$2,$8
	move	$18,$6
	move	$3,$2
	lw	$6,0x1c($2)
	lw	$7,0x20($3)
	jal	0x001b8110
	lw	$5,0x18($8)
	addu	$3,$16,$17
	.set	macro
	.set	reorder
	.end	boyAI_sub_1448C4
