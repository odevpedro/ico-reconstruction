	.text
	.p2align 3
	.globl	woodbox_init
	.ent	woodbox_init
woodbox_init:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	sd	$31,0x10($29)
	jal	0x00203aa0
	addiu	$4,$0,0xa
	lui	$2,0x29
	move	$6,$0
	addiu	$5,$2,-0x59c0
loc_0017d1f0:
	lw	$2,0($29)
	lh	$4,0($5)
	lw	$3,8($2)
	beq	$4,$3,loc_0017d218
	addiu	$6,$6,1
	sltiu	$2,$6,7
	bnez	$2,loc_0017d1f0
	addiu	$5,$5,0x30
	b	loc_0017d228
	ld	$31,0x10($29)
loc_0017d218:
	lw	$4,0($29)
	jal	0x0017bb98
	nop	
	ld	$31,0x10($29)
loc_0017d228:
	jr	$31
	addiu	$29,$29,0x20
	.set	macro
	.set	reorder
	.end	woodbox_init
