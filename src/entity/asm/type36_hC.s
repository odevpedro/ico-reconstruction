	.text
	.p2align 3
	.globl	type36_hC
	.ent	type36_hC
type36_hC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x50
	lui	$6,0x62
	sd	$17,0x10($29)
	addiu	$6,$6,-0x57b0
	sd	$19,0x30($29)
	move	$17,$4
	move	$19,$5
	lw	$4,-0x68e0($28)
	sd	$18,0x20($29)
	addiu	$5,$0,0xb0
	sd	$31,0x40($29)
	addiu	$7,$0,0x2f2
	jal	0x0013a0f8
	sd	$16,0($29)
	move	$18,$2
	lw	$3,0x15c($17)
	lui	$2,0x4c
	move	$4,$18
	addiu	$2,$2,0x6240
	sw	$18,0x800($3)
	addiu	$3,$2,0xa0
loc_001f4424:
	ld	$5,0($2)
	ld	$6,8($2)
	ld	$7,0x10($2)
	ld	$8,0x18($2)
	sd	$5,0($4)
	sd	$6,8($4)
	sd	$7,0x10($4)
	sd	$8,0x18($4)
	addiu	$2,$2,0x20
	addiu	$4,$4,0x20
	nop	
	bne	$2,$3,loc_001f4424
	nop	
	ld	$3,0($2)
	ld	$5,8($2)
	sd	$3,0($4)
	sd	$5,8($4)
	lw	$2,0x15c($17)
	lw	$3,8($2)
	blez	$3,loc_001f44a4
	move	$16,$0
	move	$5,$16
	nop	
loc_001f4480:
	move	$4,$17
	move	$6,$19
	jal	0x001f34c8
	addiu	$16,$16,1
	lw	$3,0x15c($17)
	lw	$2,8($3)
	slt	$2,$16,$2
	bnez	$2,loc_001f4480
	move	$5,$16
loc_001f44a4:
	move	$2,$18
	ld	$31,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x50
	.set	macro
	.set	reorder
	.end	type36_hC
