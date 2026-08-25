	.text
	.p2align 3
	.globl	boyAI_sub_146178
	.ent	boyAI_sub_146178
boyAI_sub_146178:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bnez	$2,loc_001461e0
	ld	$31,0x60($29)
	sw	$3,0x54($4)
	b	loc_001461e0
	swc1	$f0,0x58($4)
	jal	0x0013eb50
	addiu	$4,$0,4
	b	loc_001461ac
	move	$16,$2
	nop	
loc_001461a0:
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
loc_001461ac:
	beql	$16,$0,loc_001461d4
	lw	$2,0x164($17)
	jal	0x001653a0
	move	$4,$16
	bne	$17,$2,loc_001461a0
	move	$4,$16
	addiu	$5,$0,0xc6
	jal	0x0013ff88
	move	$6,$17
	lw	$2,0x164($17)
loc_001461d4:
	lw	$3,0x670($2)
	sw	$0,0x260($3)
	ld	$31,0x60($29)
loc_001461e0:
	ld	$20,0x50($29)
	ld	$19,0x40($29)
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x70
	nop	
	addiu	$29,$29,-0x250
	lui	$6,0xfffb
	.set	macro
	.set	reorder
	.end	boyAI_sub_146178
