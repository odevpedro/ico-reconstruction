	.text
	.p2align 3
	.globl	DispIcoMisc
	.ent	DispIcoMisc
DispIcoMisc:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x90
	sd	$31,0x60($29)
	sd	$19,0x50($29)
	sd	$18,0x40($29)
	sd	$17,0x30($29)
	sd	$16,0x20($29)
	swc1	$f23,0x88($29)
	swc1	$f22,0x80($29)
	swc1	$f21,0x78($29)
	jal	0x00129e28
	swc1	$f20,0x70($29)
	jal	0x001ebda0
	nop	
	jal	0x001e8a60
	nop	
	jal	0x001cb928
	nop	
	jal	0x0010a550
	lw	$4,-0x5c28($28)
	lw	$4,-0x5c20($28)
	jal	0x001e0978
	move	$5,$4
	lw	$2,-0x5c0c($28)
	blez	$2,loc_001afa38
	xori	$2,$2,1
	addiu	$4,$0,-0xa
	jal	0x001686d8
	movz	$4,$0,$2
loc_001afa38:
	lw	$2,-0x5c38($28)
	beqz	$2,loc_001afa50
	lw	$2,-0x5c28($28)
	jal	0x001eba40
	nop	
	lw	$2,-0x5c28($28)
loc_001afa50:
	beqz	$2,loc_001afb58
	nop	
	jal	0x00104f20
	nop	
	lui	$1,0x42c8
	mtc1	$1,$f21
	jal	0x00105278
	nop	
	lui	$1,0xc120
	mtc1	$1,$f23
	lui	$1,0x3f80
	mtc1	$1,$f22
	jal	0x00118678
	move	$4,$2
	jal	0x001118b8
	addiu	$4,$0,0xb
	addiu	$4,$0,1
	addiu	$5,$0,5
	jal	0x00111fa8
	addiu	$6,$0,0x80
	lui	$2,0x4c
	addiu	$16,$29,0x10
	addiu	$19,$2,-0x3150
	addiu	$3,$0,-0xa
loc_001afab0:
	mtc1	$3,$f0
	cvt.s.w	$f0,$f0
	addiu	$18,$3,1
	addiu	$17,$0,-0xa
	mul.s	$f20,$f0,$f21
	nop	
loc_001afac8:
	mtc1	$17,$f0
	cvt.s.w	$f0,$f0
	swc1	$f20,0($29)
	move	$4,$0
	swc1	$f23,4($29)
	move	$5,$29
	swc1	$f22,0xc($29)
	mul.s	$f0,$f0,$f21
	addiu	$17,$17,1
	jal	0x001f4be0
	swc1	$f0,8($29)
	lui	$1,0x4120
	mtc1	$1,$f12
	move	$5,$2
	jal	0x001183f0
	move	$4,$16
	move	$4,$16
	move	$5,$29
	jal	0x00118388
	move	$6,$16
	move	$4,$16
	move	$5,$19
	move	$6,$29
	move	$7,$19
	jal	0x001d4a58
	move	$8,$0
	slti	$2,$17,0xa
	bnez	$2,loc_001afac8
	move	$3,$18
	slti	$2,$3,0x14
	bnez	$2,loc_001afab0
	nop	
	jal	0x0010f630
	nop	
	jal	0x00105268
	nop	
loc_001afb58:
	jal	0x00115fb8
	nop	
	ld	$31,0x60($29)
	ld	$19,0x50($29)
	ld	$18,0x40($29)
	ld	$17,0x30($29)
	ld	$16,0x20($29)
	lwc1	$f23,0x88($29)
	lwc1	$f22,0x80($29)
	lwc1	$f21,0x78($29)
	lwc1	$f20,0x70($29)
	jr	$31
	addiu	$29,$29,0x90
	nop	
	.set	macro
	.set	reorder
	.end	DispIcoMisc
