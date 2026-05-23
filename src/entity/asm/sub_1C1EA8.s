	.text
	.p2align 3
	.globl	sub_1C1EA8
	.ent	sub_1C1EA8
sub_1C1EA8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x40
	addiu	$5,$0,0x23
	sd	$18,0x20($29)
	sd	$17,0x10($29)
	sd	$31,0x30($29)
	move	$17,$4
	sd	$16,0($29)
	lw	$2,0x15c($17)
	jal	0x00109f10
	lw	$16,0x800($2)
	move	$18,$2
	lw	$3,0($16)
	addiu	$2,$0,1
	beq	$3,$2,loc_001c1ef4
	addiu	$2,$0,2
	beql	$3,$2,loc_001c1ef8
	lw	$16,0xc($16)
	b	loc_001c1ef8
	lw	$16,4($16)
loc_001c1ef4:
	lw	$16,8($16)
loc_001c1ef8:
	jal	0x00105278
	nop	
	lw	$3,0x15c($17)
	sll	$6,$18,6
	move	$4,$2
	lw	$5,0xc($3)
	jal	0x00105f20
	addu	$5,$5,$6
	jal	0x00104f48
	addiu	$4,$0,-0x8000
	jal	0x00105278
	nop	
	lw	$4,0xc($16)
	jal	0x00105f20
	move	$5,$2
	lw	$5,0x15c($17)
	move	$4,$16
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x121d90
	addiu	$29,$29,0x40
	nop	
	.set	macro
	.set	reorder
	.end	sub_1C1EA8
