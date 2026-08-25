	.text
	.p2align 3
	.globl	ReviveAllCarryableItems
	.ent	ReviveAllCarryableItems
ReviveAllCarryableItems:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x40
	addiu	$4,$0,0x13
	sd	$17,0x10($29)
	sd	$31,0x30($29)
	sd	$18,0x20($29)
	jal	0x0013eb50
	sd	$16,0($29)
	move	$17,$2
	beqz	$17,loc_001d3e64
	addiu	$18,$0,1
	lw	$5,0x15c($17)
	nop	
loc_001d3e08:
	move	$4,$0
	lw	$2,0x16c($17)
	beqz	$2,loc_001d3e34
	lw	$3,0x800($5)
	ld	$2,8($3)
	bnez	$2,loc_001d3e34
	nop	
	lw	$2,0x48($3)
	move	$4,$18
	slt	$2,$18,$2
	movn	$4,$0,$2
loc_001d3e34:
	beqz	$4,loc_001d3e50
	nop	
	lw	$16,0x800($5)
	jal	0x0019f530
	move	$4,$17
	sw	$0,0x74($16)
	sw	$18,8($16)
loc_001d3e50:
	jal	0x0013ebe0
	move	$4,$17
	move	$17,$2
	bnel	$17,$0,loc_001d3e08
	lw	$5,0x15c($17)
loc_001d3e64:
	ld	$31,0x30($29)
	addiu	$2,$0,1
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x40
	addiu	$29,$29,-0x70
	sd	$19,0x40($29)
	swc1	$f20,0x60($29)
	move	$19,$4
	sd	$17,0x20($29)
	mul.s	$f20,$f12,$f12
	sd	$31,0x50($29)
	addiu	$4,$0,0x13
	sd	$18,0x30($29)
	jal	0x0013eb50
	sd	$16,0x10($29)
	move	$17,$2
	.word	0x12200028
	addiu	$18,$0,1
	move	$5,$17
	nop	
	jal	0x00104508
	move	$4,$29
	.word	0xdba10000
	.word	0xda620000
	.set	macro
	.set	reorder
	.end	ReviveAllCarryableItems
