	.text
	.p2align 3
	.globl	boyAI_sub_156E88
	.ent	boyAI_sub_156E88
boyAI_sub_156E88:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	ld	$2,0($4)
	sd	$2,0x10($29)
	ld	$3,8($4)
	sd	$3,0x18($29)
	ld	$2,0x10($4)
	sd	$2,0x20($29)
	ld	$3,0x18($4)
	sd	$3,0x28($29)
	ld	$2,0x20($4)
	sd	$2,0x30($29)
	ld	$3,0x28($4)
	sd	$3,0x38($29)
	lw	$2,0x15c($5)
	sw	$0,0x420($2)
	lw	$3,0($29)
	lw	$2,0x15c($3)
	lw	$4,0x4a0($2)
	beq	$4,$6,loc_00156f10
	addiu	$16,$29,0x50
	addiu	$19,$29,0x40
	addiu	$23,$0,-5
	addiu	$18,$29,0x60
	addiu	$22,$29,0x70
	addiu	$21,$29,0x80
loc_00156ee8:
	jal	0x00203aa0
	addiu	$4,$0,1
	lw	$4,0($29)
	addiu	$5,$0,0x74
	lw	$2,0x15c($4)
	lw	$3,0x4a0($2)
	bne	$3,$5,loc_00156ee8
	nop	
	.word	0x10000006
	nop	
loc_00156f10:
	addiu	$19,$29,0x40
	addiu	$23,$0,-5
	addiu	$18,$29,0x60
	.set	macro
	.set	reorder
	.end	boyAI_sub_156E88
