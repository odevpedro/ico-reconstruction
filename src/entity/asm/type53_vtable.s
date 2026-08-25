	.text
	.p2align 3
	.globl	type53_vtable
	.ent	type53_vtable
type53_vtable:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x80
	sd	$20,0x40($29)
	sd	$18,0x20($29)
	addiu	$20,$4,0x54
	sd	$31,0x70($29)
	move	$18,$0
	sd	$22,0x60($29)
	sd	$21,0x50($29)
	sd	$19,0x30($29)
	sd	$17,0x10($29)
	sd	$16,0($29)
	lw	$2,0x15c($4)
	lw	$3,4($20)
	blez	$3,loc_0019ba34
	lw	$19,0x800($2)
	addiu	$17,$4,0x5c
	addiu	$22,$0,0xd
	lui	$21,0x56
loc_0019b9e0:
	lw	$2,0($17)
	bnel	$2,$22,loc_0019ba28
	addiu	$18,$18,1
	addiu	$4,$21,-0x3960
	jal	0x001a6e28
	addiu	$16,$0,1
	sb	$16,0x10($19)
	addiu	$4,$0,0x34
	jal	0x0013eb50
	sb	$16,0x11($19)
	beqz	$2,loc_0019ba1c
	nop	
	lw	$2,0x15c($2)
	lw	$3,0x800($2)
	sb	$16,0x1a($3)
loc_0019ba1c:
	jal	0x0019c010
	addiu	$18,$18,1
	lw	$3,4($20)
loc_0019ba28:
	slt	$2,$18,$3
	bnez	$2,loc_0019b9e0
	addiu	$17,$17,8
loc_0019ba34:
	sw	$0,4($20)
	ld	$31,0x70($29)
	ld	$22,0x60($29)
	ld	$21,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x80
	.set	macro
	.set	reorder
	.end	type53_vtable
