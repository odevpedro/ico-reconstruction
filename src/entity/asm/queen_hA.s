	.text
	.p2align 3
	.globl	queen_hA
	.ent	queen_hA
queen_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	lw	$2,-0x6e0c($28)
	sd	$17,0x10($29)
	sd	$31,0x20($29)
	move	$17,$4
	beqz	$2,loc_0019a9c4
	sd	$16,0($29)
	jal	0x001504d8
	move	$4,$2
loc_0019a9c4:
	jal	0x0010ecd8
	nop	
	jal	0x0010ecb8
	move	$4,$17
	lw	$3,0x15c($17)
	lw	$16,0x800($3)
	lw	$2,0xc($16)
	beqz	$2,loc_0019aa00
	lw	$5,0x844($3)
	lw	$4,0x10($16)
	move	$6,$5
	jal	0x001c62d0
	addiu	$5,$5,0x40
	lw	$3,0x15c($17)
	lw	$5,0x844($3)
loc_0019aa00:
	lw	$4,0x14($16)
	move	$6,$5
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	addiu	$5,$5,0x40
	ld	$16,0($29)
	j	0x1c62d0
	addiu	$29,$29,0x30
	.set	macro
	.set	reorder
	.end	queen_hA
