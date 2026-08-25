	.text
	.p2align 3
	.globl	boyAI_sub_146498
	.ent	boyAI_sub_146498
boyAI_sub_146498:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$4,$18
	move	$5,$21
	jal	0x0014bdf8
	move	$4,$18
	lw	$6,0x15c($18)
	addiu	$3,$0,0x190
	addiu	$4,$22,0x5060
	lw	$2,0x4a0($6)
	lwc1	$f1,0x4ac($6)
	mult	$2,$2,$3
	addu	$2,$2,$4
	lh	$3,0x180($2)
	mtc1	$3,$f0
	cvt.s.w	$f0,$f0
	.word	0x46010034
	nop	
	bc1f	loc_001464ec
	move	$5,$6
	addiu	$2,$0,-1
	bne	$3,$2,loc_00146530
	addiu	$2,$0,1
loc_001464ec:
	lw	$2,0x4a0($5)
	addiu	$3,$0,0x190
	addiu	$4,$22,0x5060
	addiu	$5,$0,-1
	mult	$2,$2,$3
	addu	$2,$2,$4
	lh	$4,0x182($2)
	beq	$4,$5,loc_00146530
	move	$2,$0
	mtc1	$4,$f1
	cvt.s.w	$f1,$f1
	lwc1	$f0,0x4ac($6)
	.word	0x46010034
	nop	
	bc1t	loc_00146530
	addiu	$2,$0,1
	move	$2,$0
loc_00146530:
	andi	$2,$2,0xff
	.word	0x1040000e
	move	$4,$19
	addiu	$5,$16,0x47c
	addiu	$6,$0,0xf
	nop	
	lbu	$2,0($4)
	addiu	$6,$6,-1
	.set	macro
	.set	reorder
	.end	boyAI_sub_146498
