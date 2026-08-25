	.text
	.p2align 3
	.globl	boyAI_sub_150E78
	.ent	boyAI_sub_150E78
boyAI_sub_150E78:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$4,0($29)
	jal	0x0015eff8
	sw	$0,0x14($29)
	move	$5,$2
	move	$4,$22
	jal	0x00243ad0
	move	$6,$22
	lw	$4,0($29)
	move	$5,$0
	move	$6,$22
	addiu	$7,$0,1
	jal	0x0014b168
	addiu	$8,$0,1
	ld	$2,0x468($20)
	dsrl32	$2,$2,0xc
	andi	$2,$2,1
	.word	0x1040009c
	lui	$2,0x56
	ld	$2,0x478($20)
	dsrl32	$2,$2,0xc
	andi	$2,$2,1
	.word	0x10400096
	lbu	$2,-0x4c08($28)
	move	$16,$0
	bnez	$2,loc_00150f20
	move	$17,$0
	addiu	$2,$0,1
	lw	$4,-0x6e08($28)
	beqz	$4,loc_00150f64
	sb	$2,-0x4c08($28)
	jal	0x0015eff8
	nop	
	move	$5,$2
	jal	0x00195510
	move	$4,$30
	mtc1	$0,$f1
	.word	0x46000834
	nop	
	bc1t	loc_00150f1c
	addiu	$2,$0,1
	move	$2,$0
loc_00150f1c:
	sb	$2,-0x4c07($28)
loc_00150f20:
	lw	$4,-0x6e08($28)
	beqz	$4,loc_00150f68
	lw	$3,-0x6f60($28)
	jal	0x0015eff8
	addiu	$16,$0,1
	lwc1	$f0,0($2)
	addiu	$17,$0,1
	lw	$4,-0x6e08($28)
	jal	0x0015eff8
	swc1	$f0,0x20($29)
	lwc1	$f0,4($2)
	lw	$4,-0x6e08($28)
	jal	0x0015eff8
	swc1	$f0,0x24($29)
	lwc1	$f0,8($2)
	.word	0x10000013
	swc1	$f0,0x28($29)
loc_00150f64:
	lw	$3,-0x6f60($28)
loc_00150f68:
	addiu	$2,$0,0x54
	.word	0x10620005
	addiu	$2,$0,3
	.word	0x10620003
	addiu	$2,$0,0x2e
	.word	0x1462000b
	.set	macro
	.set	reorder
	.end	boyAI_sub_150E78
