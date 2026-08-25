	.text
	.p2align 3
	.globl	boyAI_sub_14BB6C
	.ent	boyAI_sub_14BB6C
boyAI_sub_14BB6C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$3,0x4a0($2)
	addiu	$4,$4,0x5060
	mult	$3,$3,$5
	addu	$3,$3,$4
	lw	$2,0x188($3)
	srl	$2,$2,0x16
	andi	$2,$2,1
	beqz	$2,loc_0014bbc8
	nop	
	lwc1	$f12,0x58($7)
	b	loc_0014bbc8
	move	$8,$0
	addiu	$5,$0,0x190
	lui	$4,0x56
	lw	$3,0x4a0($2)
	addiu	$4,$4,0x5060
	mult	$3,$3,$5
	addu	$3,$3,$4
	lw	$2,0x188($3)
	srl	$2,$2,0x15
	andi	$2,$2,1
	beql	$2,$0,loc_0014bbc8
	lwc1	$f12,0x58($7)
loc_0014bbc8:
	beqz	$8,loc_0014bbd8
	nop	
	lui	$1,0x3f80
	mtc1	$1,$f12
loc_0014bbd8:
	jal	0x0010a4e0
	move	$4,$6
	ld	$31,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	boyAI_sub_14BB6C
