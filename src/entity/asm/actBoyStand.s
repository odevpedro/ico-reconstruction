	.text
	.p2align 3
	.globl	actBoyStand
	.ent	actBoyStand
actBoyStand:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x20
	addiu	$6,$0,0x190
	sw	$4,0($29)
	sd	$31,0x10($29)
	lui	$4,0x56
	lw	$5,0($29)
	addiu	$4,$4,0x5060
	lw	$3,0x15c($5)
	lw	$2,0x4a0($3)
	mult	$2,$2,$6
	addu	$2,$2,$4
	lw	$3,0x188($2)
	srl	$3,$3,0xb
	andi	$3,$3,1
	beqz	$3,loc_00153970
	nop
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$3,0x164($2)
	lw	$5,0x678($3)
	jal	0x0015d348
	addiu	$5,$5,0x7e0
loc_00153970:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00153970
	nop
	.set	macro
	.set	reorder
	.end	actBoyStand
