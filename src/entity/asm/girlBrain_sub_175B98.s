	.text
	.p2align 3
	.globl	girlBrain_sub_175B98
	.ent	girlBrain_sub_175B98
girlBrain_sub_175B98:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
loc_00175b98:
	jal	0x00203aa0
	addiu	$4,$0,1
	lw	$2,0xd0($16)
	andi	$2,$2,0x10
	beqz	$2,loc_00175b98
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x6b90
loc_00175bb8:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x63
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00175bb8
	nop	
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x6bc0
	lw	$6,-0x6714($28)
	addiu	$5,$0,0x63
	jal	0x0013ff88
	lw	$4,-0x6e0c($28)
	jal	0x0014a3a8
	nop	
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	lw	$3,-0x6e08($28)
	lw	$3,0x164($3)
	ld	$2,0x18($3)
	dsrl32	$2,$2,4
	andi	$2,$2,1
	bnel	$2,$0,loc_00175c3c
	lw	$4,0x30($3)
	jr	$31
	addiu	$2,$0,1
loc_00175c3c:
	addiu	$3,$0,0x45
	bne	$4,$3,loc_00175c64
	move	$2,$0
	lui	$3,0x29
	addiu	$3,$3,-0x7c30
	lbu	$4,0x5d($3)
	beqz	$4,loc_00175c64
	nop	
	lbu	$2,0x58($3)
	sltiu	$2,$2,1
loc_00175c64:
	jr	$31
	nop	
	nop	
	lw	$2,-0x6e08($28)
	beqz	$2,loc_00175c88
	nop	
	lw	$2,0x164($2)
	lw	$3,0x678($2)
	sw	$4,0x3c0($3)
loc_00175c88:
	jr	$31
	nop	
	lw	$2,-0x6e08($28)
	beqz	$2,loc_00175ca8
	nop	
	lw	$2,0x164($2)
	lw	$3,0x678($2)
	sw	$0,0x3c0($3)
loc_00175ca8:
	jr	$31
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	sd	$31,0x10($29)
	move	$4,$0
	lw	$2,0($29)
	lw	$3,0x164($2)
	jal	0x00203aa0
	sw	$0,0x33c($3)
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0xa0
	lui	$2,0x28
	sw	$4,0($29)
	sd	$23,0x80($29)
	sd	$22,0x70($29)
	sd	$21,0x60($29)
	addiu	$22,$2,0x2ac0
	sd	$20,0x50($29)
	addiu	$21,$0,0xa
	sd	$19,0x40($29)
	addiu	$20,$0,0x3c
	sd	$18,0x30($29)
	addiu	$19,$0,1
	sd	$16,0x10($29)
	lui	$18,0x27
	sd	$31,0x90($29)
	addiu	$23,$18,0x4ec0
	lw	$3,0($29)
	addiu	$16,$0,1
	sd	$17,0x20($29)
	lw	$17,0x164($3)
	lw	$2,0x4ec0($18)
	addiu	$4,$0,1
	lw	$5,4($23)
	mult	$2,$2,$21
	sw	$0,0x33c($17)
	beql	$5,$0,loc_00175d50
	break	0,7
loc_00175d50:
	subu	$2,$20,$2
	div	$0,$2,$5
	mflo	$2
	srl	$3,$2,0x1f
	addu	$2,$2,$3
	sra	$2,$2,1
	div	$0,$16,$2
	mfhi	$3
	.word	0x14600002
	addiu	$16,$16,1
	.set	macro
	.set	reorder
	.end	girlBrain_sub_175B98
