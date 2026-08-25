	.text
	.p2align 3
	.globl	girlBrain_sub_16FC08
	.ent	girlBrain_sub_16FC08
girlBrain_sub_16FC08:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bc1t	loc_0016fc14
	addiu	$3,$0,1
	move	$3,$0
loc_0016fc14:
	andi	$3,$3,0xff
	bnez	$3,loc_0016fc24
	addiu	$2,$0,1
	move	$2,$0
loc_0016fc24:
	ld	$31,0x1f0($29)
	ld	$20,0x1e0($29)
	ld	$19,0x1d0($29)
	ld	$18,0x1c0($29)
	ld	$17,0x1b0($29)
	ld	$16,0x1a0($29)
	jr	$31
	addiu	$29,$29,0x200
	nop	
	lui	$2,0x27
	addiu	$7,$0,0xa
	lw	$5,0x4ec0($2)
	addiu	$29,$29,-0x190
	sd	$20,0x110($29)
	addiu	$2,$2,0x4ec0
	.word	0x00a72818
	sd	$18,0xf0($29)
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16FC08
