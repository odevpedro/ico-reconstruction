	.text
	.p2align 3
	.globl	boyAI_sub_15E91C
	.ent	boyAI_sub_15E91C
boyAI_sub_15E91C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$16,0x164($2)
	ld	$2,0x470($16)
	nop	
loc_0015e928:
	addiu	$5,$0,0xb7
	dsll	$3,$2,0x16
	dsra32	$3,$3,0
	dsll	$2,$2,0x17
	dsra32	$2,$2,0
	andi	$2,$2,1
	beqz	$2,loc_0015e960
	andi	$3,$3,1
	ld	$2,0x480($16)
	dsll	$2,$2,0x17
	dsra32	$2,$2,0
	andi	$2,$2,1
	bnez	$2,loc_0015e98c
	nop	
loc_0015e960:
	beqz	$3,loc_0015e980
	nop	
	ld	$2,0x480($16)
	dsll	$2,$2,0x16
	dsra32	$2,$2,0
	andi	$2,$2,1
	bnez	$2,loc_0015e98c
	nop	
loc_0015e980:
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
loc_0015e98c:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015e928
	ld	$2,0x470($16)
	nop	
	addiu	$29,$29,-0x40
	sw	$4,0($29)
	sd	$31,0x30($29)
	addiu	$4,$0,0xb1
	sd	$16,0x20($29)
	lw	$2,0($29)
	lw	$3,0x164($2)
	lwc1	$f1,0x190($3)
	lw	$2,0xc8($3)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15E91C
