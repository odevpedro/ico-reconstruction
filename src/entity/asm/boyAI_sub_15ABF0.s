	.text
	.p2align 3
	.globl	boyAI_sub_15ABF0
	.ent	boyAI_sub_15ABF0
boyAI_sub_15ABF0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$1,0x4248
	mtc1	$1,$f1
	.word	0x46010034
	nop	
	bc1f	loc_0015ac24
	move	$2,$0
	lwc1	$f1,0xfc($29)
	lui	$1,0x447a
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1tl	loc_0015ac24
	addiu	$2,$0,1
loc_0015ac24:
	beqz	$2,loc_0015ac8c
	lw	$4,0xf0($29)
	move	$19,$0
	lw	$2,0x15c($4)
	b	loc_0015ac90
	sw	$0,0x654($2)
	jal	0x00159df8
	move	$2,$29
	lw	$4,0x144($29)
	beqz	$4,loc_0015ac8c
	lw	$3,0xe4($29)
	ld	$2,0x20($3)
	dsll	$2,$2,0x16
	dsra32	$2,$2,0
	andi	$2,$2,1
	bnez	$2,loc_0015ac90
	lw	$4,0xf0($29)
	jal	0x0015a368
	move	$2,$29
	beqz	$2,loc_0015ac8c
	lw	$4,0xf0($29)
	addiu	$3,$0,1
	addiu	$19,$0,1
	lw	$2,0x15c($4)
	b	loc_0015ac90
	sw	$3,0x654($2)
loc_0015ac8c:
	lw	$4,0xf0($29)
loc_0015ac90:
	lw	$4,0x15c($4)
	lwc1	$f12,-0x7c78($28)
	addiu	$4,$4,0x130
	jal	0x001183f0
	move	$5,$4
	lw	$3,0xe4($29)
	ld	$2,0x20($3)
	dsll	$2,$2,0x16
	dsra32	$2,$2,0
	andi	$2,$2,1
	.word	0x1440002a
	lw	$2,-0x6f60($28)
	lw	$4,0x160($29)
	move	$5,$0
	jal	0x002641d8
	addiu	$6,$0,0x10
	lw	$2,-0x6f60($28)
	beq	$2,$23,loc_0015acec
	nop	
	beq	$2,$22,loc_0015acec
	nop	
	bne	$2,$21,loc_0015acfc
	nop	
loc_0015acec:
	lui	$1,0x4040
	mtc1	$1,$f0
	.word	0x10000004
	swc1	$f0,0x138($29)
loc_0015acfc:
	lui	$1,0x3f80
	.set	macro
	.set	reorder
	.end	boyAI_sub_15ABF0
