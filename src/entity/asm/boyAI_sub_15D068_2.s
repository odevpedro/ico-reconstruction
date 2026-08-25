	.text
	.p2align 3
	.globl	boyAI_sub_15D068_2
	.ent	boyAI_sub_15D068_2
boyAI_sub_15D068_2:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$3,0x27
	addiu	$3,$3,0x4ec0
	mult	$2,$2,$30
	lw	$5,4($3)
	lw	$3,0($29)
	beql	$5,$0,loc_0015d084
	break	0,7
loc_0015d084:
	lw	$4,0x164($3)
	subu	$2,$23,$2
	div	$0,$2,$5
	lw	$3,0x678($4)
	mflo	$2
	sw	$2,0x388($3)
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ff9a
	nop	
	nop	
	addiu	$29,$29,-0xb0
	sw	$4,0($29)
	sd	$17,0x50($29)
	lw	$4,0($29)
	addiu	$17,$29,0x20
	sd	$21,0x90($29)
	sd	$20,0x80($29)
	addiu	$21,$0,0xbb
	sd	$19,0x70($29)
	sd	$18,0x60($29)
	sd	$16,0x40($29)
	sd	$31,0xa0($29)
	jal	0x0015eff8
	move	$18,$17
	lw	$4,-0x6e08($28)
	jal	0x0015eff8
	move	$16,$2
	move	$4,$17
	move	$5,$16
	jal	0x001947d0
	.set	macro
	.set	reorder
	.end	boyAI_sub_15D068_2
