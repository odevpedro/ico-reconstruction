	.text
	.p2align 3
	.globl	enemy1_hA
	.ent	enemy1_hA
enemy1_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	sd	$31,0x10($29)
	move	$16,$4
	lw	$3,0x164($16)
	ld	$2,0x18($3)
	dsrl32	$2,$2,1
	andi	$2,$2,1
	beqz	$2,loc_001ce6e0
	ld	$31,0x10($29)
	jal	0x001bb7e0
	nop	
	jal	0x00165f88
	move	$4,$16
	bnez	$2,loc_001ce6e0
	ld	$31,0x10($29)
	move	$4,$16
	ld	$16,0($29)
	j	0x1ce5f8
	addiu	$29,$29,0x20
loc_001ce6e0:
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	enemy1_hA
