	.text
	.p2align 3
	.globl	boyAI_sub_15DE20
	.ent	boyAI_sub_15DE20
boyAI_sub_15DE20:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000fffa
	nop	
	nop	
	addiu	$29,$29,-0x20
	lw	$3,-0x6e0c($28)
	sw	$4,0($29)
	sd	$31,0x10($29)
	lw	$2,0($29)
	bne	$2,$3,loc_0015de68
	nop	
	lui	$1,0x447a
	mtc1	$1,$f12
	jal	0x00181f38
	nop	
	lw	$3,-0x6e0c($28)
loc_0015de68:
	lw	$2,0($29)
	bne	$2,$3,loc_0015de94
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
	b	loc_0015de68
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
	.set	macro
	.set	reorder
	.end	boyAI_sub_15DE20
