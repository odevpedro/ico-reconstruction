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
	beql	$2,$0,loc_001d3a80
	lw	$2,8($18)
	jal	0x001d29b8
	nop	
	b	loc_001d3ae4
	nop	
loc_001d3a80:
	beqz	$2,loc_001d3aa8
	move	$4,$17
	lw	$16,0x74($3)
	jal	0x001d2bf0
	sw	$0,0x74($3)
	beqz	$16,loc_001d3ae4
	nop	
	lw	$2,0x15c($17)
	b	loc_001d3ae4
	sw	$19,0x74($2)
loc_001d3aa8:
	lw	$16,0($3)
	beqz	$16,loc_001d3ae4
	addiu	$2,$0,0x11
	lw	$3,0xc($16)
	bne	$3,$2,loc_001d3ae4
	nop	
	jal	0x001c05a8
	move	$4,$16
	addiu	$3,$0,2
	bne	$2,$3,loc_001d3ae4
	move	$4,$17
	sw	$0,0x14($18)
	lw	$5,0x15c($16)
	jal	0x001d2738
	addiu	$5,$5,0x130
loc_001d3ae4:
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
