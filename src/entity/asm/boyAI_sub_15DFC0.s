	.text
	.p2align 3
	.globl	boyAI_sub_15DFC0
	.ent	boyAI_sub_15DFC0
boyAI_sub_15DFC0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$17,0x5ec($2)
	addiu	$5,$5,0x4a0
	jal	0x00243b18
	sw	$3,0x14($2)
	move	$4,$16
	jal	0x00106ab8
	addiu	$5,$29,0x10
	nop	
	.word	0x12200023
	nop	
	lw	$2,0($29)
	lw	$3,0x15c($2)
	lw	$4,0x604($3)
	beqz	$4,loc_0015e050
	nop	
	lw	$3,0xc($17)
	lw	$16,0($29)
	slti	$2,$3,0x16
	bnez	$2,loc_0015e03c
	slti	$2,$3,0x18
	beqz	$2,loc_0015e028
	addiu	$4,$29,0x10
	jal	0x001bc0b8
	move	$5,$17
	b	loc_0015e03c
	nop	
loc_0015e028:
	slti	$2,$3,0x1a
	beqz	$2,loc_0015e03c
	nop	
	jal	0x001bc4b8
	move	$5,$17
loc_0015e03c:
	lwc1	$f12,-0x7c24($28)
	move	$4,$16
	addiu	$5,$0,0x16
	jal	0x00102c10
	addiu	$6,$29,0x10
loc_0015e050:
	lw	$2,0($29)
	lw	$3,0x15c($2)
	lw	$4,0x600($3)
	.word	0x10800004
	nop	
	lw	$4,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15DFC0
