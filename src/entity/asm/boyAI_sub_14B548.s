	.text
	.p2align 3
	.globl	boyAI_sub_14B548
	.ent	boyAI_sub_14B548
boyAI_sub_14B548:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	b	loc_0014b564
	sd	$2,0x128($3)
	move	$4,$16
	jal	0x0013ff88
	addiu	$5,$0,0x183
	lw	$3,0x164($16)
	sw	$17,0x124($3)
loc_0014b564:
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x40
	nop	
	addiu	$29,$29,-0x20
	sd	$16,0($29)
	move	$16,$4
	sd	$31,0x10($29)
	jal	0x001e69d8
	addiu	$4,$0,1
	lw	$3,0x164($16)
	ld	$31,0x10($29)
	lw	$4,0x670($3)
	ld	$16,0($29)
	sw	$2,0x1b8($4)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0x60
	sd	$17,0x30($29)
	move	$17,$4
	.set	macro
	.set	reorder
	.end	boyAI_sub_14B548
