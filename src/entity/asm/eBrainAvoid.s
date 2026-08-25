	.text
	.p2align 3
	.globl	eBrainAvoid
	.ent	eBrainAvoid
eBrainAvoid:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x00104508
	move	$5,$18
	move	$4,$16
	jal	0x00194360
	move	$5,$29
	.word	0x46170034
	nop	
	bc1f	loc_001921b8
	lwc1	$f0,0x24($29)
	lwc1	$f1,4($29)
	sub.s	$f0,$f0,$f22
	sub.s	$f0,$f0,$f1
	.word	0x46150034
	bc1f	loc_001921a8
	nop	
	neg.s	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_001921d0
	move	$2,$0
	b	loc_001921b8
	nop	
loc_001921a8:
	.word	0x46140034
	nop	
	bc1t	loc_001921d0
	move	$2,$0
loc_001921b8:
	jal	0x0013ebe0
	move	$4,$18
	move	$18,$2
	.word	0x5640ffdc
	lw	$2,0x15c($18)
	addiu	$2,$0,1
loc_001921d0:
	ld	$31,0x60($29)
	ld	$18,0x50($29)
	ld	$17,0x40($29)
	ld	$16,0x30($29)
	lwc1	$f23,0x88($29)
	lwc1	$f22,0x80($29)
	lwc1	$f21,0x78($29)
	lwc1	$f20,0x70($29)
	jr	$31
	addiu	$29,$29,0x90
	addiu	$29,$29,-0xb0
	sd	$20,0x70($29)
	sd	$18,0x50($29)
	move	$20,$5
	move	$18,$4
	sd	$17,0x40($29)
	sd	$16,0x30($29)
	move	$4,$29
	sd	$31,0x80($29)
	sd	$19,0x60($29)
	swc1	$f23,0xa8($29)
	swc1	$f22,0xa0($29)
	swc1	$f21,0x98($29)
	jal	0x00104508
	swc1	$f20,0x90($29)
	move	$17,$29
	jal	0x0013eb50
	addiu	$4,$0,0x11
	move	$16,$2
	.word	0x1200002a
	addiu	$2,$0,1
	lwc1	$f23,-0x7948($28)
	lui	$1,0x4248
	mtc1	$1,$f22
	mtc1	$0,$f21
	lui	$1,0x42c8
	mtc1	$1,$f20
	lw	$2,0x16c($16)
	.word	0x1040001b
	addiu	$4,$29,0x10
	jal	0x00104508
	move	$5,$16
	move	$4,$17
	jal	0x00194360
	addiu	$5,$29,0x10
	.word	0x46170034
	nop	
	.word	0x45000012
	lwc1	$f1,0x14($29)
	lwc1	$f0,4($17)
	sub.s	$f0,$f0,$f22
	sub.s	$f0,$f0,$f1
	.word	0x46150034
	bc1f	loc_001922c8
	nop	
	neg.s	$f0,$f0
	.word	0x46140034
	nop	
	.word	0x4501000d
	move	$2,$0
	.word	0x10000005
	nop	
loc_001922c8:
	.word	0x46140034
	nop	
	.word	0x45010007
	move	$2,$0
	.set	macro
	.set	reorder
	.end	eBrainAvoid
