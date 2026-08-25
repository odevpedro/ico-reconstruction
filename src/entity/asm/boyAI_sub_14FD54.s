	.text
	.p2align 3
	.globl	boyAI_sub_14FD54
	.ent	boyAI_sub_14FD54
boyAI_sub_14FD54:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x8a
	b	loc_0014fd9c
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x8b
	b	loc_0014fd9c
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x8c
	b	loc_0014fd9c
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x8d
loc_0014fd9c:
	lw	$4,0($29)
	addiu	$5,$0,0x85
	lw	$2,0x15c($4)
	lw	$3,0x4a0($2)
	beq	$3,$5,loc_0014fdc8
	nop	
	lw	$2,0x2d0($18)
	mtc1	$0,$f20
	andi	$2,$2,0x20
	beqz	$2,loc_0014fdd4
	lw	$16,-0x6e0c($28)
loc_0014fdc8:
	lui	$1,0x3f80
	mtc1	$1,$f20
	lw	$16,-0x6e0c($28)
loc_0014fdd4:
	lui	$2,0x28
	addiu	$17,$2,0x1d60
	jal	0x0015ef88
	move	$4,$16
	lwc1	$f0,0($2)
	lui	$3,0x28
	move	$4,$16
	jal	0x0015ef88
	swc1	$f0,0x1d60($3)
	lwc1	$f0,4($2)
	move	$4,$16
	jal	0x0015ef88
	swc1	$f0,4($17)
	lwc1	$f0,8($2)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14FD54
