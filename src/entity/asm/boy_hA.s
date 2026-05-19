	.text
	.p2align 3
	.globl	boy_hA
	.ent	boy_hA
boy_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x60
	sd	$17,0x30($29)
	sd	$31,0x50($29)
	move	$17,$4
	sd	$18,0x40($29)
	sd	$16,0x20($29)
	lw	$2,0x15c($17)
	jal	0x001e4938
	lw	$18,0x800($2)
	lw	$3,0x10($18)
	beqz	$3,0f
	move	$4,$29
	jal	0x00104508
	move	$5,$17
	addiu	$16,$29,0x10
	move	$5,$17
	jal	0x00102850
	move	$4,$16
	move	$4,$16
	jal	0x0010e158
	addiu	$5,$0,-0x8000
	lwc1	$f12,0x14($18)
	cvt.s.w	$f12,$f12
	move	$6,$16
	addiu	$4,$0,0x1a8
	jal	0x0012a060
	move	$5,$29
	cvt.w.s	$f1,$f0
	mfc1	$4,$f1
	lui	$2,0x27
	lw	$3,0x4ed4($2)
	bnez	$3,2f
	lw	$3,-0x6f60($28)
	addiu	$2,$0,-1
	bnel	$4,$2,1f
	sw	$4,0x14($18)
	b	3f
	addiu	$2,$0,0x27
0:
	jal	0x0010ecd8
	nop	
	jal	0x0010ecb8
	move	$4,$17
	jal	0x001c1ea8
	move	$4,$17
	jal	0x001c1250
	move	$4,$17
1:
	lw	$3,-0x6f60($28)
2:
	addiu	$2,$0,0x27
3:
	bne	$3,$2,4f
	ld	$31,0x50($29)
	lw	$4,0x15c($17)
	lui	$1,0x41a0
	mtc1	$1,$f1
	lwc1	$f0,0x644($4)
	.word	0x46000834
	nop	
	bc1f	5f
	ld	$18,0x40($29)
	lw	$4,0x648($4)
	beql	$4,$0,6f
	ld	$17,0x30($29)
	jal	0x0010d180
	nop	
	bnez	$2,4f
	ld	$31,0x50($29)
	lw	$2,0x15c($17)
	move	$6,$17
	lw	$16,0x800($2)
	lw	$5,0x648($2)
	addiu	$16,$16,0x2c
	jal	0x0010cb48
	move	$4,$16
	jal	0x0010cf88
	move	$4,$16
	ld	$31,0x50($29)
4:
	ld	$18,0x40($29)
5:
	ld	$17,0x30($29)
6:
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0x60
	.set	macro
	.set	reorder
	.end	boy_hA
