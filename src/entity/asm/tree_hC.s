	.text
	.p2align 3
	.globl	tree_hC
	.ent	tree_hC
tree_hC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x70
	lui	$6,0x62
	sd	$18,0x40($29)
	addiu	$6,$6,-0x5928
	sd	$31,0x60($29)
	addiu	$7,$0,0x23
	sd	$19,0x50($29)
	sd	$17,0x30($29)
	sd	$16,0x20($29)
	lw	$2,0x15c($4)
	lw	$4,-0x68e0($28)
	lw	$3,0x820($2)
	lb	$16,0x2e($3)
	jal	0x0013a0f8
	sll	$5,$16,1
	blez	$16,loc_001f1814
	move	$18,$2
	move	$17,$16
	move	$16,$18
	nop	
loc_001f1800:
	jal	0x00264d60
	addiu	$17,$17,-1
	sh	$2,0($16)
	bnez	$17,loc_001f1800
	addiu	$16,$16,2
loc_001f1814:
	move	$2,$18
	ld	$31,0x60($29)
	ld	$19,0x50($29)
	ld	$18,0x40($29)
	ld	$17,0x30($29)
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0x70
	.set	macro
	.set	reorder
	.end	tree_hC
