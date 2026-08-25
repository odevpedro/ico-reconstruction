	.text
	.p2align 3
	.globl	boyAI_sub_15DE70
	.ent	boyAI_sub_15DE70
boyAI_sub_15DE70:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,-0x6e08($28)
	beqz	$2,loc_0015de94
	nop	
	jal	0x00182be8
	nop	
	lw	$6,0($29)
	addiu	$5,$0,0x3a
	jal	0x0013ff88
	lw	$4,-0x6e08($28)
loc_0015de94:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ffef
	lw	$3,-0x6e0c($28)
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x7728
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$5,0x164($2)
	jal	0x00106ab8
	addiu	$5,$5,0x1b0
	lw	$4,0($29)
	jal	0x00157dc8
	nop	
	lw	$4,0($29)
	addiu	$5,$0,3
	lw	$2,0x164($4)
	lw	$3,0x670($2)
	lw	$4,0x1dc($3)
	bne	$4,$5,loc_0015df10
	nop	
	jal	0x00203aa0
	addiu	$4,$0,0x168
	nop	
loc_0015df10:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015df10
	nop	
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15DE70
