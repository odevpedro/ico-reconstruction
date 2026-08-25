	.text
	.p2align 3
	.globl	boyAI_sub_14D140
	.ent	boyAI_sub_14D140
boyAI_sub_14D140:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$4,$18
	jal	0x0015eff8
	swc1	$f0,0x40($29)
	lwc1	$f0,4($2)
	move	$4,$18
	jal	0x0015eff8
	swc1	$f0,0x44($29)
	lwc1	$f0,8($2)
	move	$4,$18
	jal	0x0015ef88
	swc1	$f0,0x48($29)
	move	$5,$2
	jal	0x001940b0
	move	$4,$20
	lwc1	$f2,0($29)
	move	$5,$21
	lwc1	$f1,4($29)
	move	$6,$16
	lwc1	$f0,8($29)
	move	$4,$19
	swc1	$f2,0x50($29)
	swc1	$f1,0x54($29)
	jal	0x00243ae8
	swc1	$f0,0x58($29)
	move	$4,$19
	move	$5,$20
	sw	$0,0x6c($29)
	jal	0x002438b8
	move	$6,$4
	lw	$3,0x15c($18)
	lui	$2,0x56
	addiu	$5,$2,0x5060
	addiu	$4,$0,0x190
	lw	$2,0x4a0($3)
	mult	$2,$2,$4
	addu	$2,$2,$5
	lw	$3,0x18c($2)
	srl	$3,$3,3
	andi	$3,$3,1
	.word	0x5460002b
	move	$4,$18
	bnez	$22,loc_0014d1f4
	lwc1	$f1,0x68($29)
	.word	0x13c00027
	move	$4,$18
loc_0014d1f4:
	mtc1	$0,$f0
	.word	0x46000834
	bc1f	loc_0014d228
	nop	
	neg.s	$f1,$f1
	lui	$1,0x4316
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0014d240
	addiu	$16,$29,0x20
	.word	0x1000001a
	move	$4,$18
loc_0014d228:
	lui	$1,0x4316
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	.word	0x45000013
	addiu	$16,$29,0x20
loc_0014d240:
	move	$4,$18
	move	$5,$0
	addiu	$6,$0,2
	addiu	$7,$0,1
	.set	macro
	.set	reorder
	.end	boyAI_sub_14D140
