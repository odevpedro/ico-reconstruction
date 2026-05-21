	.text
	.p2align 3
	.globl	ItemGeo
	.ent	ItemGeo
ItemGeo:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x50
	sd	$19,0x30($29)
	sd	$17,0x10($29)
	addiu	$19,$0,1
	sd	$31,0x40($29)
	move	$17,$4
	sd	$18,0x20($29)
	sd	$16,0($29)
	lw	$3,0x15c($17)
	lw	$18,0x800($3)
	lw	$2,0($18)
	.word	0x1053002c
	ld	$31,0x40($29)
	lw	$2,0xc($18)
	beql	$2,$0,0f
	lw	$2,8($18)
	jal	0x001d29b8
	nop	
	b	2f
	nop	
0:
	beqz	$2,1f
	move	$4,$17
	lw	$16,0x74($3)
	jal	0x001d2bf0
	sw	$0,0x74($3)
	beqz	$16,2f
	nop	
	lw	$2,0x15c($17)
	b	2f
	sw	$19,0x74($2)
1:
	lw	$16,0($3)
	beqz	$16,2f
	addiu	$2,$0,0x11
	lw	$3,0xc($16)
	bne	$3,$2,2f
	nop	
	jal	0x001c05a8
	move	$4,$16
	addiu	$3,$0,2
	bne	$2,$3,2f
	move	$4,$17
	sw	$0,0x14($18)
	lw	$5,0x15c($16)
	jal	0x001d2738
	addiu	$5,$5,0x130
2:
	jal	0x00102858
	move	$4,$17
	lw	$2,0x15c($17)
	lw	$4,0x800($2)
	lw	$3,4($4)
	xori	$3,$3,1
	.word	0x14600004
	ld	$31,0x40($29)
	jal	0x001d37c8
	move	$4,$17
	ld	$31,0x40($29)
	.set	macro
	.set	reorder
	.end	ItemGeo
