	.text
	.p2align 3
	.globl	ap1_hA
	.ent	ap1_hA
ap1_hA:
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
	lw	$16,0x800($3)
	lw	$2,8($16)
	slti	$2,$2,5
	beqz	$2,loc_001ba5ac
	ld	$31,0x20($29)
	lw	$2,0x278($16)
	beql	$2,$0,loc_001ba5b0
	ld	$17,0x10($29)
	jal	0x0010ecd8
	nop	
	jal	0x0010ecb8
	move	$4,$17
	lw	$2,4($16)
	bnel	$2,$0,loc_001ba598
	lw	$4,0x19c($16)
	jal	0x0010eca0
	lw	$4,0x194($16)
	jal	0x0010eca0
	lw	$4,0x198($16)
	lw	$4,0x19c($16)
loc_001ba598:
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x1cf998
	addiu	$29,$29,0x30
loc_001ba5ac:
	ld	$17,0x10($29)
loc_001ba5b0:
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	.set	macro
	.set	reorder
	.end	ap1_hA
