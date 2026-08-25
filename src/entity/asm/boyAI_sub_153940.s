	.text
	.p2align 3
	.globl	boyAI_sub_153940
	.ent	boyAI_sub_153940
boyAI_sub_153940:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
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
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	sd	$31,0x10($29)
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$3,0x164($2)
	lw	$5,0x678($3)
	jal	0x0015d348
	addiu	$5,$5,0x7e0
	jal	0x00203aa0
	move	$4,$0
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	sd	$31,0x10($29)
	lw	$2,0($29)
	lw	$4,0($29)
	lw	$3,0x164($2)
	lw	$5,0x678($3)
	jal	0x0015d348
	addiu	$5,$5,0x7e0
	jal	0x00203aa0
	move	$4,$0
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	addiu	$29,$29,-0x30
	sw	$4,0($29)
	sd	$31,0x20($29)
	lui	$4,0x56
	lw	$2,0($29)
	addiu	$4,$4,-0x79c0
	sd	$16,0x10($29)
	jal	0x001a6e28
	lw	$16,0x164($2)
	addiu	$2,$0,5
	sw	$2,0x30($16)
	nop	
loc_00153a20:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00153a20
	.set	macro
	.set	reorder
	.end	boyAI_sub_153940
