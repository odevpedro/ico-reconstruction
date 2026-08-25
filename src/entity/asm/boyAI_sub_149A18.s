	.text
	.p2align 3
	.globl	boyAI_sub_149A18
	.ent	boyAI_sub_149A18
boyAI_sub_149A18:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bc1f	loc_00149a48
	lw	$3,0($29)
	lw	$2,0x170($3)
	bnez	$2,loc_00149aac
	lw	$2,-0x6e0c($28)
	lw	$4,0x174($3)
	beqz	$4,loc_00149aa8
	sw	$4,0x170($3)
	jal	0x001d2550
	nop	
	b	loc_00149aa8
	lw	$5,4($29)
loc_00149a48:
	jal	0x00149768
	move	$2,$29
	b	loc_00149aa8
	lw	$5,4($29)
	lw	$3,0($29)
	lw	$2,0x170($3)
	bnez	$2,loc_00149aa8
	lw	$5,4($29)
	lw	$4,0x174($3)
	beqz	$4,loc_00149aa4
	sw	$4,0x170($3)
	jal	0x001d2550
	lw	$5,4($29)
	b	loc_00149aa8
	lw	$5,4($29)
	lw	$2,0($29)
	lw	$4,0x170($2)
	beqz	$4,loc_00149aa8
	lw	$5,4($29)
	.word	0x1000ffd3
	nop	
	b	loc_00149aa8
	move	$5,$8
loc_00149aa4:
	lw	$5,4($29)
loc_00149aa8:
	lw	$2,-0x6e0c($28)
loc_00149aac:
	bne	$5,$2,loc_00149ad0
	ld	$31,0x30($29)
	lw	$2,0($29)
	lw	$3,0x170($2)
	lw	$4,0x140($2)
	move	$5,$3
	jal	0x00154ce8
	sw	$3,0x144($2)
	ld	$31,0x30($29)
loc_00149ad0:
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0x40
	nop	
	addiu	$29,$29,-0x70
	lw	$5,-0x6e0c($28)
	sd	$31,0x50($29)
	sd	$17,0x40($29)
	sd	$16,0x30($29)
	.word	0x10a00046
	swc1	$f20,0x60($29)
	lw	$2,-0x6e08($28)
	.word	0x10400044
	ld	$31,0x50($29)
	jal	0x00104508
	move	$4,$29
	addiu	$17,$29,0x10
	lw	$5,-0x6e08($28)
	jal	0x00104508
	move	$4,$17
	move	$4,$29
	jal	0x00194398
	move	$5,$17
	lwc1	$f1,-0x7e04($28)
	.word	0x46010034
	nop	
	.word	0x45010036
	ld	$31,0x50($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_149A18
