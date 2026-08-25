	.text
	.p2align 3
	.globl	boyAI_sub_15D5B0
	.ent	boyAI_sub_15D5B0
boyAI_sub_15D5B0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$4,0($29)
	lw	$5,0x164($2)
	jal	0x00104360
	addiu	$5,$5,0x160
	lw	$4,0($29)
	jal	0x001ce778
	nop	
	nop	
loc_0015d5d0:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xd2
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015d5d0
	nop	
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x7740
	jal	0x00203aa0
	move	$4,$0
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0x20
	addiu	$2,$0,0x182
	sw	$4,0($29)
	sd	$31,0x10($29)
	lw	$3,0($29)
	lw	$7,0x164($3)
	lw	$4,0xc8($7)
	.word	0x14820014
	nop	
	lw	$2,0x128($7)
	andi	$2,$2,1
	.word	0x10400010
	lui	$3,0x27
	addiu	$6,$0,0xa
	lw	$5,0x4ec0($3)
	addiu	$2,$0,0x3c
	addiu	$3,$3,0x4ec0
	.set	macro
	.set	reorder
	.end	boyAI_sub_15D5B0
