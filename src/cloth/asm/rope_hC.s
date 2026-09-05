	.text
	.globl	rope_hC
	.ent	rope_hC
rope_hC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x40
	addiu	$5,$0,1
	sd	$18,0x20($29)
	sd	$31,0x30($29)
	move	$18,$4
	sd	$17,0x10($29)
	sd	$16,0($29)
	lw	$2,0x15c($18)
	lw	$4,0x800($2)
	lw	$3,0($4)
	beq	$3,$5,loc_001d3bdc
	ld	$31,0x30($29)
	lw	$2,4($4)
	xori	$2,$2,1
	bnel	$2,$0,loc_001d3bc8
	move	$4,$18
	addiu	$16,$4,0x40
	lw	$17,8($16)
	slti	$2,$17,2
	beql	$2,$0,loc_001d3be0
	ld	$18,0x20($29)
	bnel	$17,$5,loc_001d3bc8
	move	$4,$18
	lui	$2,0x27
	lw	$3,0x4ed4($2)
	beqz	$3,loc_001d3bac
	lw	$2,0x24($16)
	beqz	$2,loc_001d3bc8
	move	$4,$18
	jal	0x001d12d8
	move	$4,$18
	b	loc_001d3bc0
	sw	$0,0x24($16)
loc_001d3bac:
	bnez	$2,loc_001d3bc4
	move	$4,$18
	jal	0x001d2540
	move	$4,$18
	sw	$17,0x24($16)
loc_001d3bc0:
	move	$4,$18
loc_001d3bc4:
	ld	$31,0x30($29)
loc_001d3bc8:
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x10ecb8
	addiu	$29,$29,0x40
loc_001d3bdc:
	ld	$18,0x20($29)
loc_001d3be0:
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x40
	.set	macro
	.set	reorder
	.end	rope_hC
