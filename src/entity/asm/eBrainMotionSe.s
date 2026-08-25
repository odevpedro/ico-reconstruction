	.text
	.p2align 3
	.globl	eBrainMotionSe
	.ent	eBrainMotionSe
eBrainMotionSe:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,4($4)
	addu	$2,$2,$5
	slti	$3,$2,0x1801
	bnez	$3,loc_00191f0c
	sw	$2,4($4)
	addiu	$2,$0,0x1800
	sw	$2,4($4)
loc_00191f0c:
	addiu	$16,$17,0xd20
	lw	$4,0xd20($17)
	jal	0x00140b60
	lw	$5,4($16)
	lw	$2,4($16)
	bnez	$2,loc_00191f34
	ld	$31,0x60($29)
	addiu	$2,$0,2
	sw	$2,-0x616c($28)
	ld	$31,0x60($29)
loc_00191f34:
	ld	$20,0x50($29)
	ld	$19,0x40($29)
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x70
	addiu	$29,$29,-0x20
	lw	$3,-0x616c($28)
	sd	$31,0x10($29)
	addiu	$2,$0,1
	.word	0x10620010
	sd	$16,0($29)
	.set	macro
	.set	reorder
	.end	eBrainMotionSe
