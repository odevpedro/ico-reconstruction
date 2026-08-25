	.text
	.p2align 3
	.globl	boy_hB
	.ent	boy_hB
boy_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	sd	$16,0($29)
	swc1	$f21,0x28($29)
	swc1	$f20,0x20($29)
	sd	$31,0x10($29)
	jal	0x001d23e0
	move	$16,$4
	lui	$1,0x4248
	mtc1	$1,$f20
	lwc1	$f21,-0x771c($28)
	jal	0x001e3fc8
	move	$4,$16
	jal	0x001c1c48
	move	$4,$16
	jal	0x001c12f0
	move	$4,$16
	jal	0x001e4868
	move	$4,$16
	addiu	$7,$0,0x198
	move	$4,$16
	addiu	$5,$0,0x23
	jal	0x001bb8c0
	addiu	$6,$0,0x2c
	move	$4,$16
	addiu	$5,$0,4
	mov.s	$f12,$f20
	mov.s	$f13,$f20
	jal	0x00103d50
	mov.s	$f14,$f21
	beqz	$2,loc_001c1e60
	move	$4,$16
	addiu	$5,$0,6
	jal	0x0013ff88
	move	$6,$16
loc_001c1e60:
	jal	0x0014a0d8
	nop	
	lui	$1,0x41f0
	mtc1	$1,$f12
	beqz	$2,loc_001c1e80
	move	$4,$16
	lui	$1,0x4170
	mtc1	$1,$f12
loc_001c1e80:
	mov.s	$f13,$f20
	mov.s	$f14,$f21
	ld	$31,0x10($29)
	ld	$16,0($29)
	addiu	$5,$0,2
	lwc1	$f21,0x28($29)
	lwc1	$f20,0x20($29)
	j	0x103d50
	addiu	$29,$29,0x30
	nop	
	addiu	$29,$29,-0x40
	addiu	$5,$0,0x23
	sd	$18,0x20($29)
	sd	$17,0x10($29)
	sd	$31,0x30($29)
	move	$17,$4
	sd	$16,0($29)
	lw	$2,0x15c($17)
	jal	0x00109f10
	lw	$16,0x800($2)
	move	$18,$2
	lw	$3,0($16)
	addiu	$2,$0,1
	beq	$3,$2,loc_001c1ef4
	addiu	$2,$0,2
	beql	$3,$2,loc_001c1ef8
	lw	$16,0xc($16)
	b	loc_001c1ef8
	lw	$16,4($16)
loc_001c1ef4:
	lw	$16,8($16)
loc_001c1ef8:
	jal	0x00105278
	nop	
	lw	$3,0x15c($17)
	sll	$6,$18,6
	move	$4,$2
	lw	$5,0xc($3)
	jal	0x00105f20
	addu	$5,$5,$6
	jal	0x00104f48
	addiu	$4,$0,-0x8000
	jal	0x00105278
	nop	
	lw	$4,0xc($16)
	jal	0x00105f20
	move	$5,$2
	lw	$5,0x15c($17)
	move	$4,$16
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x121d90
	addiu	$29,$29,0x40
	nop	
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
	beqz	$3,loc_001c1ff0
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
	bnez	$3,loc_001c2014
	lw	$3,-0x6f60($28)
	addiu	$2,$0,-1
	bnel	$4,$2,loc_001c2010
	sw	$4,0x14($18)
	b	loc_001c2018
	addiu	$2,$0,0x27
loc_001c1ff0:
	jal	0x0010ecd8
	nop	
	jal	0x0010ecb8
	move	$4,$17
	jal	0x001c1ea8
	move	$4,$17
	jal	0x001c1250
	move	$4,$17
loc_001c2010:
	lw	$3,-0x6f60($28)
loc_001c2014:
	addiu	$2,$0,0x27
loc_001c2018:
	bne	$3,$2,loc_001c2084
	ld	$31,0x50($29)
	lw	$4,0x15c($17)
	lui	$1,0x41a0
	mtc1	$1,$f1
	lwc1	$f0,0x644($4)
	.word	0x46000834
	nop	
	bc1f	loc_001c2088
	ld	$18,0x40($29)
	lw	$4,0x648($4)
	beql	$4,$0,loc_001c208c
	ld	$17,0x30($29)
	jal	0x0010d180
	nop	
	bnez	$2,loc_001c2084
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
loc_001c2084:
	ld	$18,0x40($29)
loc_001c2088:
	ld	$17,0x30($29)
loc_001c208c:
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0x60
	.set	macro
	.set	reorder
	.end	boy_hB
