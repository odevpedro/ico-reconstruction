	.text
	.p2align 3
	.globl	world_state_load
	.ent	world_state_load
world_state_load:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$3,-0x6f60($28)
	addiu	$4,$0,0x194
	lui	$5,0x5f
	mult	$3,$3,$4
	addiu	$2,$5,0x2fb8
	addu	$2,$2,$3
	lw	$2,0x154($2)
	beqz	$2,loc_001af974
	nop	
	jalr	$2
	nop	
loc_001af974:
	jal	0x00166028
	nop	
	jal	0x001ae3e8
	nop	
	lui	$3,0x27
	ld	$31,0xa0($29)
	addiu	$2,$3,0x4ec0
	ld	$30,0x90($29)
	ld	$23,0x80($29)
	move	$4,$0
	ld	$22,0x70($29)
	ld	$21,0x60($29)
	ld	$20,0x50($29)
	ld	$19,0x40($29)
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	sw	$0,0x18($2)
	sw	$0,0x14($2)
	j	0x13d3f8
	addiu	$29,$29,0xb0
	.set	macro
	.set	reorder
	.end	world_state_load
