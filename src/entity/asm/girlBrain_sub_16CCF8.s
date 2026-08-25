	.text
	.p2align 3
	.globl	girlBrain_sub_16CCF8
	.ent	girlBrain_sub_16CCF8
girlBrain_sub_16CCF8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	and	$6,$2,$5
	sd	$6,0x20($20)
	lw	$5,0x70($29)
	bne	$5,$4,loc_0016cd28
	move	$9,$0
	addiu	$2,$23,0x2ac0
	lw	$3,0xc90($2)
	beqz	$3,loc_0016cd28
	addiu	$2,$0,0x20
	or	$2,$6,$2
	sd	$2,0x20($20)
	lw	$5,0x70($29)
loc_0016cd28:
	addiu	$3,$5,-2
	sltiu	$2,$3,8
	beqz	$2,loc_0016cd74
	sll	$2,$3,2
	lui	$4,0x56
	addiu	$3,$4,-0x6a00
	addu	$2,$2,$3
	lw	$4,0($2)
	jr	$4
	nop	
	ld	$2,0x20($20)
	addiu	$3,$0,0x10
	addiu	$7,$0,1
	b	loc_0016cd80
	or	$2,$2,$3
	ld	$2,0x20($20)
	addiu	$3,$0,0x10
	b	loc_0016cd80
	or	$2,$2,$3
loc_0016cd74:
	ld	$2,0x20($20)
	addiu	$3,$0,-0x11
	and	$2,$2,$3
loc_0016cd80:
	sd	$2,0x20($20)
	lw	$3,-0x63ec($28)
	beqz	$3,loc_0016cda0
	addiu	$2,$0,1
	.word	0x1062001a
	nop	
	.word	0x1000001c
	nop	
loc_0016cda0:
	lw	$2,-0x63f0($28)
	addiu	$8,$2,1
	.word	0x10e00018
	sw	$8,-0x63f0($28)
	lw	$6,0x4ec0($22)
	addiu	$3,$0,0xa
	addiu	$5,$22,0x4ec0
	addiu	$2,$0,0x3c
	.word	0x00c33018
	lw	$4,4($5)
	addiu	$7,$0,0x1e
	addiu	$3,$0,1
	move	$9,$3
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16CCF8
