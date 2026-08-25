	.text
	.p2align 3
	.globl	boyAI_sub_151F8C
	.ent	boyAI_sub_151F8C
boyAI_sub_151F8C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$4,0($29)
	addiu	$16,$29,0x10
	jal	0x0015eff8
	swc1	$f0,0x18($29)
	move	$5,$16
	move	$6,$2
	jal	0x001947d0
	move	$4,$17
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$17
	lw	$4,0($29)
	addiu	$5,$0,0xe2
	lw	$2,0x15c($4)
	lw	$3,0x4a0($2)
	.word	0x14650030
	nop	
	lw	$3,0($29)
	lui	$1,0x4170
	mtc1	$1,$f1
	lw	$2,0x15c($3)
	lwc1	$f0,0x4ac($2)
	.word	0x46000834
	bc1f	loc_00152024
	nop	
	lw	$2,0($29)
	lw	$3,0x15c($2)
	lwc1	$f0,0x4ac($3)
	.word	0x46140034
	bc1f	loc_00152024
	nop	
	bnez	$21,loc_00152024
	addiu	$2,$20,-0x5520
	lw	$5,0($29)
	lw	$4,0x24($2)
	addiu	$6,$0,6
	jal	0x001f3f58
	addiu	$21,$0,1
loc_00152024:
	lw	$3,0($29)
	lui	$1,0x41f0
	mtc1	$1,$f1
	lw	$2,0x15c($3)
	lwc1	$f0,0x4ac($2)
	.word	0x46000834
	bc1f	loc_00152074
	nop	
	bnez	$19,loc_00152074
	addiu	$16,$20,-0x5520
	lw	$4,0x20($16)
	beqz	$4,loc_0015206c
	nop	
	jal	0x001f4098
	nop	
	lw	$4,0x20($16)
	jal	0x001d12a8
	addiu	$5,$0,0x4c
loc_0015206c:
	jal	0x001502d8
	addiu	$19,$0,1
loc_00152074:
	lw	$2,0($29)
	lw	$3,0x15c($2)
	lwc1	$f0,0x4ac($3)
	.word	0x4600a034
	.set	macro
	.set	reorder
	.end	boyAI_sub_151F8C
