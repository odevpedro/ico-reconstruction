	.text
	.p2align 3
	.globl	eBrainSetFlag
	.ent	eBrainSetFlag
eBrainSetFlag:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	lui	$2,0x11
	addiu	$29,$29,-0x70
	ori	$2,$2,1
	sd	$19,0x40($29)
	sd	$17,0x20($29)
	move	$4,$29
	sd	$31,0x60($29)
	lui	$17,0x6d
	sd	$20,0x50($29)
	move	$19,$0
	sd	$18,0x30($29)
	sd	$16,0x10($29)
	jal	0x00143cd0
	sw	$2,0($29)
	addiu	$4,$17,0xd20
	bnez	$2,loc_00191dc4
	sw	$2,0xd20($17)
	lw	$3,-0x6170($28)
	addiu	$2,$0,1
	.word	0x1062005c
	sw	$0,4($4)
loc_00191dc4:
	lui	$20,0x27
	addiu	$16,$20,0x4ec0
	lw	$3,0x18($16)
	.word	0x14600058
	ld	$31,0x60($29)
	lw	$2,0x14($16)
	.word	0x5440001a
	sw	$0,-0x6168($28)
	lw	$2,-0x6170($28)
	addiu	$18,$0,1
	.word	0x10520015
	.set	macro
	.set	reorder
	.end	eBrainSetFlag
