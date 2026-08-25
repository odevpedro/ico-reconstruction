	.text
	.p2align 3
	.globl	boyAI_sub_14BDD0
	.ent	boyAI_sub_14BDD0
boyAI_sub_14BDD0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$2,$2,-0x5698
	nop	
	addiu	$2,$0,1
	addiu	$3,$0,9
	movn	$2,$3,$4
	ori	$4,$2,4
	jr	$31
	movn	$2,$4,$5
	jr	$31
	nop	
	addiu	$29,$29,-0x90
	sd	$18,0x60($29)
	sd	$17,0x50($29)
	move	$18,$4
	sd	$31,0x70($29)
	move	$17,$5
	swc1	$f20,0x80($29)
	sd	$16,0x40($29)
	jal	0x0015eff8
	lw	$16,0x164($18)
	lwc1	$f0,0($2)
	move	$4,$18
	jal	0x0015eff8
	swc1	$f0,0($29)
	lwc1	$f0,4($2)
	move	$4,$18
	jal	0x0015eff8
	swc1	$f0,4($29)
	lwc1	$f0,8($2)
	ori	$5,$0,0x8000
	.set	macro
	.set	reorder
	.end	boyAI_sub_14BDD0
