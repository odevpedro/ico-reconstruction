	.text
	.p2align 3
	.globl	girlBrain_sub_16CFC8
	.ent	girlBrain_sub_16CFC8
girlBrain_sub_16CFC8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lwc1	$f0,0x60($29)
	lwc1	$f1,0x64($29)
	lwc1	$f2,0x68($29)
	lw	$5,-0x6680($28)
	swc1	$f0,0x20($29)
	swc1	$f1,0x24($29)
	beqz	$5,loc_0016d008
	swc1	$f2,0x28($29)
	lw	$3,0xc($5)
	addiu	$2,$0,0x11
	bne	$3,$2,loc_0016d00c
	lwc1	$f0,0x24($29)
	lw	$2,0($29)
	lw	$3,0x164($2)
	lw	$4,0x678($3)
	sw	$5,0x374($4)
loc_0016d008:
	lwc1	$f0,0x24($29)
loc_0016d00c:
	lui	$1,0x4120
	mtc1	$1,$f1
	sub.s	$f0,$f0,$f1
	swc1	$f0,0x24($29)
	lw	$2,-0x6e0c($28)
	addiu	$4,$0,0x55
	lw	$2,0x164($2)
	lw	$3,0x30($2)
	bne	$3,$4,loc_0016d04c
	nop	
	lwc1	$f0,0x500($2)
	swc1	$f0,0x20($29)
	lwc1	$f1,0x504($2)
	swc1	$f1,0x24($29)
	lwc1	$f0,0x508($2)
	swc1	$f0,0x28($29)
loc_0016d04c:
	lw	$4,0($29)
	move	$5,$20
	move	$6,$19
	move	$7,$0
	move	$8,$0
	jal	0x00200eb0
	move	$9,$0
	bnez	$2,loc_0016d080
	addiu	$2,$18,0x110
loc_0016d070:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0016d070
	nop	
loc_0016d080:
	lui	$23,0x6b
	mtc1	$0,$f22
	move	$17,$23
	sw	$2,0xa0($29)
	addiu	$30,$29,0x70
	move	$22,$19
	addiu	$21,$29,0x80
	addiu	$20,$29,0x90
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	lwc1	$f0,0($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x70($29)
	lwc1	$f0,4($2)
	lw	$4,0($29)
	jal	0x0015eff8
	swc1	$f0,0x74($29)
	lwc1	$f0,8($2)
	move	$6,$22
	lw	$5,0xa0($29)
	move	$7,$0
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16CFC8
