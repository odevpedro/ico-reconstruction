	.text
	.p2align 3
	.globl	boyAI_sub_15EA7C
	.ent	boyAI_sub_15EA7C
boyAI_sub_15EA7C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	lui	$4,0x56
	sd	$31,0x10($29)
	jal	0x001a6e28
	addiu	$4,$4,-0x73c8
	jal	0x00203aa0
	move	$4,$0
	ld	$31,0x10($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0xb0
	sw	$4,0($29)
	lw	$4,0($29)
	sd	$21,0x70($29)
	sd	$20,0x60($29)
	sd	$19,0x50($29)
	sd	$18,0x40($29)
	addiu	$19,$0,0x12
	sd	$17,0x30($29)
	move	$18,$0
	sd	$16,0x20($29)
	swc1	$f23,0xa8($29)
	swc1	$f22,0xa0($29)
	swc1	$f21,0x98($29)
	swc1	$f20,0x90($29)
	sd	$31,0x80($29)
	jal	0x0015ef88
	move	$17,$0
	lui	$1,0x3f80
	mtc1	$1,$f23
	lwc1	$f22,-0x7c20($28)
	jal	0x00194630
	move	$4,$2
	lwc1	$f1,-0x7c1c($28)
	addiu	$16,$29,0x10
	lui	$1,0x4334
	mtc1	$1,$f2
	nop	
	nop	
	div.s	$f0,$f0,$f1
	lw	$2,0($29)
	mov.s	$f21,$f1
	mov.s	$f20,$f2
	lw	$21,0x164($2)
	mul.s	$f0,$f0,$f2
	cvt.w.s	$f1,$f0
	mfc1	$20,$f1
	move	$4,$16
	move	$5,$0
	addiu	$6,$0,0x10
	jal	0x002641d8
	addiu	$17,$17,1
	addu	$4,$20,$18
	jal	0x00194960
	swc1	$f23,0x18($29)
	addiu	$18,$18,5
	mtc1	$2,$f12
	cvt.s.w	$f12,$f12
	mul.s	$f12,$f12,$f21
	nop	
	nop	
	div.s	$f12,$f12,$f20
	jal	0x001945b8
	.set	macro
	.set	reorder
	.end	boyAI_sub_15EA7C
