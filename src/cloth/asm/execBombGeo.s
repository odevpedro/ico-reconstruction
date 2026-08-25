	.text
	.p2align 3
	.globl	execBombGeo
	.ent	execBombGeo
execBombGeo:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x60
	sd	$18,0x30($29)
	sd	$17,0x20($29)
	move	$18,$4
	sd	$31,0x50($29)
	sd	$19,0x40($29)
	sd	$16,0x10($29)
	lw	$2,0x15c($18)
	lw	$19,0x800($2)
	addiu	$17,$19,0x40
	lw	$3,8($17)
	sltiu	$2,$3,5
	beqz	$2,loc_001d3818
	lui	$2,0x62
	sll	$3,$3,2
	addiu	$2,$2,-0x7050
	addu	$3,$3,$2
	lw	$4,0($3)
	jr	$4
	nop	
loc_001d3818:
	jal	0x001f2148
	lw	$4,0($17)
	.word	0x1040007b
	addiu	$16,$0,1
	.set	macro
	.set	reorder
	.end	execBombGeo
