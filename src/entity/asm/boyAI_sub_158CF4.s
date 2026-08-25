	.text
	.p2align 3
	.globl	boyAI_sub_158CF4
	.ent	boyAI_sub_158CF4
boyAI_sub_158CF4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,0x34($18)
	addiu	$30,$0,1
	bne	$2,$30,loc_00158d90
	lw	$3,0x108($29)
	lw	$2,0($29)
	move	$4,$23
	move	$5,$17
	jal	0x00104508
	lw	$16,0x164($2)
	addiu	$4,$29,0x30
	jal	0x00104508
	move	$5,$17
	lui	$1,0xc1c8
	mtc1	$1,$f12
	addiu	$5,$16,0x4a0
	jal	0x00243b18
	addiu	$4,$29,0xe0
	addiu	$4,$29,0x30
	addiu	$6,$29,0xe0
	jal	0x00243ad0
	move	$5,$4
	lwc1	$f2,0x24($29)
	move	$4,$23
	lwc1	$f0,0x34($29)
	add.s	$f2,$f2,$f22
	lui	$1,0x41f0
	mtc1	$1,$f1
	add.s	$f0,$f0,$f22
	swc1	$f1,0x70($23)
	swc1	$f2,0x24($29)
	jal	0x001683c8
	swc1	$f0,0x34($29)
	lw	$2,0xa8($29)
	move	$3,$0
	movn	$3,$30,$2
	.word	0x54600022
	addiu	$21,$0,1
	lw	$2,0x34($18)
	lw	$3,0x108($29)
loc_00158d90:
	bne	$2,$3,loc_00158db4
	addiu	$4,$29,0x20
	lwc1	$f0,0x4a0($18)
	swc1	$f0,0x20($29)
	lwc1	$f1,0x4a4($18)
	swc1	$f1,0x24($29)
	lwc1	$f0,0x4a8($18)
	b	loc_00158dc4
	swc1	$f0,0x28($29)
loc_00158db4:
	lui	$1,0xbf80
	mtc1	$1,$f12
	jal	0x00243b18
	lw	$5,0x10c($29)
loc_00158dc4:
	bnez	$20,loc_00158ddc
	nop	
	lui	$1,0x42c8
	mtc1	$1,$f12
	jal	0x001bd278
	move	$4,$17
loc_00158ddc:
	lw	$6,0($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_158CF4
