	.text
	.p2align 3
	.globl	boyAI_sub_14BB0C
	.ent	boyAI_sub_14BB0C
boyAI_sub_14BB0C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$3,0x670($2)
	swc1	$f0,0x58($3)
	jr	$31
	sw	$0,0x54($3)
	nop	
	addiu	$29,$29,-0x10
	move	$6,$4
	sd	$31,0($29)
	lui	$1,0x3f80
	mtc1	$1,$f12
	lw	$2,0x164($6)
	lw	$7,0x670($2)
	lw	$3,0x54($7)
	sltiu	$2,$3,3
	beqz	$2,loc_0014bb58
	move	$8,$0
	lw	$2,-0x6e08($28)
	xor	$2,$6,$2
	sltiu	$8,$2,1
loc_0014bb58:
	addiu	$2,$0,1
	bne	$3,$2,loc_0014bb9c
	lw	$2,0x15c($6)
	addiu	$5,$0,0x190
	lui	$4,0x56
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
loc_0014bb9c:
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
	addiu	$29,$29,0x10
	nop	
	addiu	$29,$29,-0x30
	sd	$16,0x10($29)
	move	$16,$4
	.set	macro
	.set	reorder
	.end	boyAI_sub_14BB0C
