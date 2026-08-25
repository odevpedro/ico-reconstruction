	.text
	.p2align 3
	.globl	type49_hB
	.ent	type49_hB
type49_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	sd	$17,0x10($29)
	sd	$31,0x20($29)
	move	$17,$4
	sd	$16,0($29)
	lw	$3,0x15c($17)
	sw	$17,-0x5474($28)
	lw	$2,0x8c($3)
	beqz	$2,loc_001e0964
	sw	$2,-0x5478($28)
	jal	0x00105278
	move	$16,$0
	jal	0x00243bd8
	move	$4,$2
	jal	0x00104f48
	addiu	$4,$0,-0x8000
	lw	$4,0x15c($17)
	jal	0x001e0708
	move	$5,$0
	lw	$5,0x15c($17)
	lw	$2,0x88($5)
	blez	$2,loc_001e0948
	lw	$2,-0x5c28($28)
	lw	$4,0xc($5)
loc_001e0918:
	sll	$2,$16,6
	addiu	$5,$5,0x20
	addiu	$16,$16,1
	addu	$4,$4,$2
	jal	0x002438e8
	move	$6,$4
	lw	$5,0x15c($17)
	lw	$2,0x88($5)
	slt	$2,$16,$2
	bnel	$2,$0,loc_001e0918
	lw	$4,0xc($5)
	lw	$2,-0x5c28($28)
loc_001e0948:
	beqz	$2,loc_001e0968
	ld	$31,0x20($29)
	move	$4,$17
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x1e0808
	addiu	$29,$29,0x30
loc_001e0964:
	ld	$31,0x20($29)
loc_001e0968:
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	.set	macro
	.set	reorder
	.end	type49_hB
