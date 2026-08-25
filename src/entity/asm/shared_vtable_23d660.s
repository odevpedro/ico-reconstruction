	.text
	.p2align 3
	.globl	shared_vtable_23d660
	.ent	shared_vtable_23d660
shared_vtable_23d660:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	j	0x203d90
	nop	
	bltz	$4,loc_0023d6dc
	addiu	$3,$0,0x4c
	lui	$2,0x2a
	mult	$3,$4,$3
	addiu	$2,$2,0x4c48
	addu	$2,$2,$3
	lw	$4,0x34($2)
	beqz	$4,loc_0023d6a0
	addiu	$3,$0,0x14
	lui	$2,0x2a
	mult	$3,$4,$3
	addiu	$2,$2,-0xd90
	b	loc_0023d6a4
	addu	$4,$3,$2
loc_0023d6a0:
	move	$4,$0
loc_0023d6a4:
	beqz	$4,loc_0023d6dc
	addiu	$3,$0,1
	lw	$2,0x10($4)
	andi	$2,$2,1
	bnel	$2,$3,loc_0023d6c8
	lw	$3,0xc($4)
	addiu	$2,$0,0x32f
	sw	$2,0xc($4)
	lw	$3,0xc($4)
loc_0023d6c8:
	addiu	$2,$0,0x32f
	bne	$3,$2,loc_0023d6dc
	nop	
	lw	$2,0($4)
	sw	$2,0xc($4)
loc_0023d6dc:
	jr	$31
	nop	
	.set	macro
	.set	reorder
	.end	shared_vtable_23d660
