	.text
	.p2align 3
	.globl	boyAI_sub_144EB4
	.ent	boyAI_sub_144EB4
boyAI_sub_144EB4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lwc1	$f1,0x34($2)
	swc1	$f1,4($29)
	lwc1	$f0,0x38($2)
	jal	0x001943c8
	swc1	$f0,8($29)
	.word	0x46140034
	nop	
	.word	0x45010052
	addiu	$2,$0,1
	slti	$2,$19,0x168
	.word	0x1040004e
	addiu	$2,$0,4
	lw	$3,0xc($16)
	bne	$3,$2,loc_00144f00
	sw	$0,0x10($29)
	lui	$1,0xbf80
	mtc1	$1,$f0
	b	loc_00144f0c
	swc1	$f0,0x14($29)
loc_00144f00:
	lui	$1,0x3f80
	mtc1	$1,$f0
	swc1	$f0,0x14($29)
loc_00144f0c:
	sw	$0,0x18($29)
	lw	$2,0x15c($16)
	addiu	$17,$29,0x10
	sw	$0,0x1c($29)
	move	$4,$17
	move	$6,$17
	move	$21,$17
	lw	$5,0xc($2)
	jal	0x002438b8
	.set	macro
	.set	reorder
	.end	boyAI_sub_144EB4
