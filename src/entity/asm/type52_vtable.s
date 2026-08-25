	.text
	.p2align 3
	.globl	type52_vtable
	.ent	type52_vtable
type52_vtable:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0xa0
	sd	$19,0x30($29)
	sd	$18,0x20($29)
	sd	$17,0x10($29)
	move	$18,$4
	sd	$31,0x90($29)
	addiu	$19,$18,0x54
	sd	$30,0x80($29)
	move	$17,$0
	sd	$23,0x70($29)
	sd	$22,0x60($29)
	sd	$21,0x50($29)
	sd	$20,0x40($29)
	sd	$16,0($29)
	lw	$2,0x15c($18)
	lw	$3,4($19)
	blez	$3,loc_0019bc20
	lw	$20,0x800($2)
	lui	$30,0x56
	addiu	$23,$0,5
	lui	$22,0x56
	addiu	$21,$0,1
	addiu	$16,$18,0x5c
	nop	
loc_0019bbc0:
	lw	$5,0($16)
	addiu	$2,$0,0xd
	beq	$5,$2,loc_0019bbe0
	nop	
	jal	0x001a6e28
	addiu	$4,$30,-0x3928
	b	loc_0019bc10
	lw	$2,4($19)
loc_0019bbe0:
	jal	0x0017e5b0
	nop	
	bnel	$2,$23,loc_0019bc10
	lw	$2,4($19)
	jal	0x001a6e28
	addiu	$4,$22,-0x3918
	sb	$21,0x18($20)
	addiu	$5,$0,0x18b
	move	$6,$18
	jal	0x0013ff88
	lw	$4,-0x6e0c($28)
	lw	$2,4($19)
loc_0019bc10:
	addiu	$17,$17,1
	slt	$2,$17,$2
	bnez	$2,loc_0019bbc0
	addiu	$16,$16,8
loc_0019bc20:
	sw	$0,4($19)
	ld	$31,0x90($29)
	ld	$30,0x80($29)
	ld	$23,0x70($29)
	ld	$22,0x60($29)
	ld	$21,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0xa0
	.set	macro
	.set	reorder
	.end	type52_vtable
