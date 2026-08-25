	.text
	.p2align 3
	.globl	boyAI_sub_158B68
	.ent	boyAI_sub_158B68
boyAI_sub_158B68:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	lui	$1,0x447a
	mtc1	$1,$f12
	jal	0x00181f38
	nop	
	lw	$6,0($29)
	move	$5,$17
	jal	0x001bf460
	lw	$4,0x110($29)
	.word	0x16800049
	nop	
	lw	$2,0($29)
	addiu	$4,$29,0x30
	move	$5,$17
	lui	$1,0x4120
	mtc1	$1,$f20
	lw	$16,0x164($2)
	jal	0x00104508
	addiu	$19,$0,1
	lui	$1,0x4240
	mtc1	$1,$f21
	addiu	$4,$29,0x40
	jal	0x00104508
	move	$5,$17
	lui	$1,0x42c8
	mtc1	$1,$f12
	addiu	$5,$16,0x4a0
	jal	0x00243b18
	addiu	$4,$29,0xf0
	addiu	$4,$29,0x40
	addiu	$6,$29,0xf0
	jal	0x00243ad0
	move	$5,$4
	lwc1	$f1,0x34($29)
	addiu	$4,$29,0x30
	lwc1	$f0,0x44($29)
	add.s	$f1,$f1,$f20
	swc1	$f21,0xa0($29)
	add.s	$f0,$f0,$f20
	swc1	$f1,0x34($29)
	jal	0x001683c8
	swc1	$f0,0x44($29)
	lw	$2,0xb8($29)
	beql	$2,$0,loc_00158c30
	lw	$2,0x34($18)
	lw	$4,0($29)
	addiu	$5,$0,0xb7
	jal	0x0015bcc8
	addiu	$21,$0,1
	lw	$2,0x34($18)
loc_00158c30:
	lw	$3,0x108($29)
	.word	0x1443001f
	.set	macro
	.set	reorder
	.end	boyAI_sub_158B68
