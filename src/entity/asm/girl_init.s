	.text
	.p2align 3
	.globl	girl_init
	.ent	girl_init
girl_init:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x70
	addiu	$6,$0,0x1e
	sd	$17,0x10($29)
	lui	$17,0x27
	sd	$19,0x30($29)
	lw	$2,0x4ec0($17)
	addiu	$19,$0,0xa
	sd	$21,0x50($29)
	mult	$2,$2,$19
	sd	$20,0x40($29)
	sd	$18,0x20($29)
	move	$20,$4
	sd	$16,0($29)
	addiu	$18,$17,0x4ec0
	sd	$31,0x60($29)
	addiu	$16,$0,0x3c
	subu	$2,$16,$2
	lui	$4,0x56
	lw	$3,4($18)
	addiu	$4,$4,-0x64e8
	sw	$0,-0x63e8($28)
	move	$5,$20
	div	$0,$2,$3
	sw	$0,-0x63ec($28)
	beql	$3,$0,loc_00174c08
	break	0,7
loc_00174c08:
	mflo	$2
	mult	$2,$2,$6
	jal	0x001a6e28
	sw	$2,-0x63f0($28)
	jal	0x00202208
	move	$4,$20
	move	$21,$2
	jal	0x00202148
	move	$4,$20
	jal	0x00203918
	move	$4,$20
	jal	0x00203910
	move	$4,$20
	jal	0x0014b358
	addiu	$4,$0,0x20
	lw	$3,0x4ec0($17)
	lw	$2,4($18)
	mult	$3,$3,$19
	lui	$1,0x4270
	mtc1	$1,$f2
	lw	$5,0x164($20)
	beql	$2,$0,loc_00174c64
	break	0,7
loc_00174c64:
	lw	$6,0x670($5)
	subu	$16,$16,$3
	div	$0,$16,$2
	mflo	$16
	mtc1	$16,$f1
	cvt.s.w	$f1,$f1
	mul.s	$f0,$f0,$f1
	nop	
	nop	
	div.s	$f0,$f0,$f2
	cvt.w.s	$f1,$f0
	swc1	$f1,0x254($6)
	jal	0x0014b580
	move	$4,$20
	jal	0x0014b150
	move	$4,$20
	jal	0x0014b1d0
	move	$4,$20
	jal	0x0014b260
	move	$4,$20
	jal	0x00203aa0
	addiu	$4,$0,1
	jal	0x0014adf8
	move	$4,$20
	lw	$5,-0x6e0c($28)
	jal	0x00182958
	move	$4,$20
	lw	$2,-0x5bf0($28)
	beqz	$2,loc_00174ce8
	addiu	$5,$0,0x14
	lui	$4,0x17
	jal	0x00203b78
	addiu	$4,$4,-0x4208
loc_00174ce8:
	lui	$16,0x29
	lui	$4,0x17
	addiu	$16,$16,-0x2ba8
	addiu	$4,$4,0x1e48
	sw	$16,0xc0($21)
	jal	0x00203b78
	addiu	$5,$0,0x15
	lui	$4,0x17
	addiu	$5,$0,0x15
	jal	0x00203b78
	addiu	$4,$4,0x25c8
	lui	$4,0x16
	addiu	$5,$0,0x15
	jal	0x00203b78
	addiu	$4,$4,-0x3838
	lui	$1,0x42c8
	mtc1	$1,$f0
	addiu	$16,$16,0x78
	sw	$0,0x340($21)
	addiu	$2,$0,1
	sw	$16,0xc4($21)
	move	$4,$20
	sw	$2,0x44($21)
	addiu	$5,$0,0xb7
	jal	0x0015bcc8
	swc1	$f0,0x1d0($21)
	ld	$31,0x60($29)
	move	$4,$0
	ld	$21,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x203aa0
	addiu	$29,$29,0x70
	addiu	$29,$29,-0x70
	addiu	$5,$0,0xfd
	sd	$18,0x50($29)
	move	$18,$4
	sd	$31,0x60($29)
	sd	$17,0x40($29)
	jal	0x0015bcc8
	sd	$16,0x30($29)
	lw	$5,-0x6e0c($28)
	addiu	$2,$0,1
	lw	$3,0x164($5)
	lw	$4,0x30($3)
	beq	$4,$2,loc_00174ee8
	addiu	$17,$29,0x10
	jal	0x00104508
	move	$4,$17
	addiu	$16,$29,0x20
	move	$5,$18
	jal	0x00104508
	move	$4,$16
	move	$6,$16
	move	$5,$17
	jal	0x00243ae8
	move	$4,$29
	move	$4,$29
	jal	0x00243978
	move	$5,$29
	jal	0x0015ef88
	move	$4,$18
	move	$5,$2
	jal	0x00194508
	move	$4,$29
	mtc1	$2,$f2
	cvt.s.w	$f2,$f2
	mtc1	$0,$f0
	.word	0x46001034
	bc1f	loc_00174e34
	nop	
	neg.s	$f1,$f2
	lui	$1,0x4234
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_00174e4c
	move	$4,$18
	b	loc_00174e5c
	nop	
loc_00174e34:
	lui	$1,0x4234
	mtc1	$1,$f0
	.word	0x46001034
	nop	
	bc1f	loc_00174e5c
	move	$4,$18
loc_00174e4c:
	jal	0x0015bcc8
	addiu	$5,$0,0xfe
	b	loc_00174eec
	ld	$31,0x60($29)
loc_00174e5c:
	mtc1	$0,$f0
	.word	0x46001034
	bc1f	loc_00174e90
	nop	
	neg.s	$f1,$f2
	lui	$1,0x4307
	mtc1	$1,$f0
	.word	0x46010034
	nop	
	bc1t	loc_00174ea8
	move	$4,$18
	b	loc_00174eb8
	nop	
loc_00174e90:
	lui	$1,0x4307
	mtc1	$1,$f0
	.word	0x46020034
	nop	
	bc1f	loc_00174eb8
	move	$4,$18
loc_00174ea8:
	jal	0x0015bcc8
	addiu	$5,$0,0xff
	b	loc_00174eec
	ld	$31,0x60($29)
loc_00174eb8:
	lui	$1,0x4234
	mtc1	$1,$f0
	.word	0x46020034
	nop	
	bc1f	loc_00174ee0
	move	$4,$18
	jal	0x0015bcc8
	addiu	$5,$0,0x100
	b	loc_00174eec
	ld	$31,0x60($29)
loc_00174ee0:
	jal	0x0015bcc8
	addiu	$5,$0,0x101
loc_00174ee8:
	ld	$31,0x60($29)
loc_00174eec:
	ld	$18,0x50($29)
	ld	$17,0x40($29)
	ld	$16,0x30($29)
	jr	$31
	addiu	$29,$29,0x70
	.set	macro
	.set	reorder
	.end	girl_init
