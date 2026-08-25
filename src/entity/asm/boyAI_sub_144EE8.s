	.text
	.p2align 3
	.globl	boyAI_sub_144EE8
	.ent	boyAI_sub_144EE8
boyAI_sub_144EE8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
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
	addu	$5,$5,$18
	addiu	$18,$29,0x20
	move	$5,$20
	move	$4,$18
	jal	0x00243ae8
	move	$6,$29
	lwc1	$f1,0x14($29)
	mtc1	$0,$f0
	.word	0x46000834
	nop	
	bc1f	loc_00144f80
	move	$20,$18
	neg.s	$f1,$f1
	lwc1	$f0,-0x7e28($28)
	.word	0x46010034
	nop	
	bc1t	loc_00144f94
	srl	$2,$19,0x1f
	.word	0x10000014
	move	$4,$21
loc_00144f80:
	lwc1	$f0,-0x7e24($28)
	.word	0x46010034
	nop	
	.word	0x4500000e
	srl	$2,$19,0x1f
loc_00144f94:
	jal	0x0015ef88
	.set	macro
	.set	reorder
	.end	boyAI_sub_144EE8
