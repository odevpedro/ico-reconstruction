	.text
	.p2align 3
	.globl	girl_hB
	.ent	girl_hB
girl_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x50
	sd	$17,0x20($29)
	move	$17,$4
	swc1	$f20,0x40($29)
	sd	$31,0x30($29)
	jal	0x001d23e0
	sd	$16,0x10($29)
	lui	$1,0x4248
	mtc1	$1,$f20
	jal	0x001e3fc8
	move	$4,$17
	addiu	$7,$0,0x197
	move	$4,$17
	addiu	$5,$0,0x23
	jal	0x001bb8c0
	addiu	$6,$0,0x2c
	mtc1	$0,$f14
	move	$4,$17
	addiu	$5,$0,4
	mov.s	$f12,$f20
	jal	0x00103d50
	mov.s	$f13,$f20
	beqz	$2,loc_001d1864
	move	$4,$17
	addiu	$5,$0,6
	jal	0x0013ff88
	move	$6,$17
loc_001d1864:
	jal	0x0014a0d8
	nop	
	lui	$1,0x41f0
	mtc1	$1,$f12
	beqz	$2,loc_001d1884
	nop	
	lui	$1,0x4170
	mtc1	$1,$f12
loc_001d1884:
	lwc1	$f14,-0x7670($28)
	move	$4,$17
	addiu	$5,$0,1
	jal	0x00103d50
	mov.s	$f13,$f20
	beqz	$2,loc_001d18bc
	move	$4,$17
	addiu	$5,$0,6
	jal	0x0013ff88
	move	$6,$17
	jal	0x00174d78
	move	$4,$17
	b	loc_001d199c
	nop	
loc_001d18bc:
	lw	$4,0x15c($17)
	addiu	$3,$0,4
	lw	$2,0x310($4)
	bne	$2,$3,loc_001d199c
	nop	
	lw	$2,0x7c($4)
	beqz	$2,loc_001d199c
	nop	
	lw	$2,0x3c8($4)
	beqz	$2,loc_001d199c
	lw	$4,-0x6e0c($28)
	jal	0x00109f10
	addiu	$5,$0,6
	move	$16,$2
	move	$4,$17
	jal	0x00109f10
	addiu	$5,$0,0x16
	lw	$4,-0x6e0c($28)
	sll	$2,$2,6
	lw	$5,0x15c($17)
	sll	$16,$16,6
	lw	$3,0x15c($4)
	lw	$6,0xc($5)
	move	$4,$29
	lw	$5,0xc($3)
	addu	$6,$6,$2
	addu	$5,$5,$16
	addiu	$6,$6,0x30
	jal	0x00243ae8
	addiu	$5,$5,0x30
	move	$4,$29
	jal	0x00243950
	move	$5,$29
	jal	0x00105fe0
	mov.s	$f12,$f0
	mov.s	$f1,$f0
	lui	$1,0x4120
	mtc1	$1,$f0
	.word	0x46010034
	bc1f	loc_001d199c
	nop	
	nop	
	nop	
	div.s	$f1,$f1,$f20
	lui	$1,0x3f80
	mtc1	$1,$f0
	lui	$1,0x3f40
	mtc1	$1,$f2
	sub.s	$f0,$f0,$f1
	.word	0x46020034
	nop	
	bc1tl	loc_001d1990
	mov.s	$f0,$f2
loc_001d1990:
	lw	$4,-0x6e0c($28)
	jal	0x0010a4e0
	mov.s	$f12,$f0
loc_001d199c:
	jal	0x001d14b8
	move	$4,$17
	ld	$31,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	lwc1	$f20,0x40($29)
	jr	$31
	addiu	$29,$29,0x50
	.set	macro
	.set	reorder
	.end	girl_hB
