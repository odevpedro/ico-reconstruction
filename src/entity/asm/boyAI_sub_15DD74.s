	.text
	.p2align 3
	.globl	boyAI_sub_15DD74
	.ent	boyAI_sub_15DD74
boyAI_sub_15DD74:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	addiu	$29,$29,-0x30
	sw	$4,0($29)
	sd	$31,0x20($29)
	lui	$4,0x56
	lw	$2,0($29)
	addiu	$4,$4,-0x7420
	sd	$16,0x10($29)
	jal	0x001a6e28
	lw	$16,0x164($2)
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$5,0x164($2)
	jal	0x00106ab8
	addiu	$5,$5,0x1b0
	lw	$3,0($29)
	addiu	$4,$0,4
	lw	$2,0xc($3)
	bne	$2,$4,loc_0015dde0
	lw	$4,-0x6e0c($28)
	lw	$4,0($29)
	jal	0x00157da0
	addiu	$5,$0,6
	lw	$4,0($29)
	jal	0x0014b5b8
	nop	
	lw	$4,-0x6e0c($28)
loc_0015dde0:
	addiu	$5,$0,0x10c
	lw	$6,0($29)
	jal	0x0013ff88
	nop	
	lw	$3,0xc8($16)
	addiu	$2,$0,0x10a
	bne	$3,$2,loc_0015de18
	nop	
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	lw	$4,0($29)
	jal	0x00154e60
	move	$5,$2
loc_0015de18:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015de18
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
	.word	0x14430009
	lw	$2,-0x6e08($28)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15DD74
