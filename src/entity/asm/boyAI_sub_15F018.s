	.text
	.p2align 3
	.globl	boyAI_sub_15F018
	.ent	boyAI_sub_15F018
boyAI_sub_15F018:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bnez	$2,loc_0015f034
	slti	$2,$3,3
	addiu	$2,$0,0x2b
	beq	$3,$2,loc_0015f054
	lui	$16,0x6b
	b	loc_0015f090
	move	$5,$4
loc_0015f034:
	beqz	$2,loc_0015f08c
	lui	$16,0x6b
	blez	$3,loc_0015f090
	move	$5,$4
	lw	$16,0x164($4)
	move	$5,$4
	b	loc_0015f094
	addiu	$16,$16,0xf0
loc_0015f054:
	lui	$17,0x6b
	move	$6,$4
	addiu	$16,$17,-0x5490
	move	$5,$29
	jal	0x001c2e28
	move	$4,$16
	bnez	$2,loc_0015f0a0
	move	$2,$16
	lui	$2,0x63
	lwc1	$f0,0x2340($2)
	swc1	$f0,-0x5490($17)
	swc1	$f0,8($16)
	b	loc_0015f09c
	swc1	$f0,4($16)
loc_0015f08c:
	move	$5,$4
loc_0015f090:
	addiu	$16,$16,-0x5490
loc_0015f094:
	jal	0x00104508
	move	$4,$16
loc_0015f09c:
	move	$2,$16
loc_0015f0a0:
	ld	$31,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x40
	nop	
	addiu	$29,$29,-0x60
	.set	macro
	.set	reorder
	.end	boyAI_sub_15F018
