	.text
	.p2align 3
	.globl	boyAI_sub_14B344
	.ent	boyAI_sub_14B344
boyAI_sub_14B344:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	sw	$5,0x54($3)
	swc1	$f12,0x58($3)
	jr	$31
	nop	
	lui	$2,0x29
	sll	$4,$4,2
	addiu	$2,$2,0x24b0
	addu	$4,$4,$2
	jr	$31
	lwc1	$f0,0($4)
	lw	$3,0x15c($4)
	addiu	$6,$0,0x190
	lui	$5,0x56
	lw	$7,0x164($4)
	lw	$2,0x4a0($3)
	addiu	$5,$5,0x5060
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
	beqz	$3,loc_0014b428
	move	$2,$0
	ld	$3,0x478($7)
	dsrl32	$3,$3,0xc
	andi	$3,$3,1
	beqz	$3,loc_0014b428
	nop	
	lw	$4,0x30($7)
	sltiu	$3,$4,4
	beqz	$3,loc_0014b428
	nop	
	addiu	$2,$0,2
	movz	$2,$0,$4
loc_0014b428:
	jr	$31
	nop	
	lw	$6,0x15c($4)
	lui	$7,0x56
	addiu	$4,$0,0x190
	addiu	$3,$7,0x5060
	lw	$2,0x4a0($6)
	lwc1	$f1,0x4ac($6)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14B344
