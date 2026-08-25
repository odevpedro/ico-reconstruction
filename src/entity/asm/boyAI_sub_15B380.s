	.text
	.p2align 3
	.globl	boyAI_sub_15B380
	.ent	boyAI_sub_15B380
boyAI_sub_15B380:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$4,$0,0x26
	lw	$2,0x164($2)
	lw	$3,0x30($2)
	bne	$3,$4,loc_0015b3e0
	nop	
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	lw	$4,-0x6e08($28)
	jal	0x0015eff8
	move	$16,$2
	move	$4,$16
	jal	0x00194398
	move	$5,$2
	lui	$1,0x4561
	mtc1	$1,$f1
	.word	0x46010034
	nop	
	bc1f	loc_0015b3e0
	lw	$5,-0x6e0c($28)
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x31
	lw	$5,-0x6e0c($28)
loc_0015b3e0:
	lw	$2,0($29)
	beq	$2,$5,loc_0015b3fc
	nop	
	lw	$3,0($29)
	lw	$2,-0x6e08($28)
	.word	0x1462002b
	nop	
loc_0015b3fc:
	lw	$4,0($29)
	lui	$1,0x45c8
	mtc1	$1,$f20
	jal	0x0015eff8
	addiu	$17,$0,0x26
	lwc1	$f0,0($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x10($29)
	lwc1	$f0,4($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x14($29)
	lwc1	$f0,8($2)
	addiu	$4,$0,4
	jal	0x0013eb50
	swc1	$f0,0x18($29)
	b	loc_0015b454
	move	$16,$2
loc_0015b448:
	jal	0x0013ebe0
	move	$4,$16
	move	$16,$2
loc_0015b454:
	.word	0x12000013
	nop	
	lw	$2,0x16c($16)
	beqz	$2,loc_0015b448
	nop	
	lw	$2,0x164($16)
	lw	$3,0x30($2)
	bne	$3,$17,loc_0015b448
	move	$4,$16
	jal	0x0015eff8
	nop	
	move	$5,$2
	jal	0x00194398
	move	$4,$30
	.set	macro
	.set	reorder
	.end	boyAI_sub_15B380
