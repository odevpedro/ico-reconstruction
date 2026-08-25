	.text
	.p2align 3
	.globl	boyAI_sub_14B388
	.ent	boyAI_sub_14B388
boyAI_sub_14B388:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$4,$0,2
	mult	$2,$2,$6
	addu	$2,$2,$5
	lw	$3,0x184($2)
	srl	$3,$3,0x13
	andi	$3,$3,7
	beq	$3,$4,loc_0014b3dc
	slti	$2,$3,3
	beqz	$2,loc_0014b3c0
	addiu	$2,$0,1
	beq	$3,$2,loc_0014b3d4
	nop	
	b	loc_0014b3ec
	ld	$3,0x468($7)
loc_0014b3c0:
	addiu	$2,$0,3
	beq	$3,$2,loc_0014b3e4
	nop	
	b	loc_0014b3ec
	ld	$3,0x468($7)
loc_0014b3d4:
	jr	$31
	addiu	$2,$0,1
loc_0014b3dc:
	jr	$31
	addiu	$2,$0,2
loc_0014b3e4:
	jr	$31
	move	$2,$0
loc_0014b3ec:
	dsrl32	$3,$3,0xc
	andi	$3,$3,1
	.word	0x1060000c
	.set	macro
	.set	reorder
	.end	boyAI_sub_14B388
