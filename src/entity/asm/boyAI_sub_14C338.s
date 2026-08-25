	.text
	.p2align 3
	.globl	boyAI_sub_14C338
	.ent	boyAI_sub_14C338
boyAI_sub_14C338:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x00194508
	addiu	$5,$29,0x10
	bgez	$2,loc_0014c374
	move	$4,$17
	jal	0x00194508
	addiu	$5,$29,0x10
	negu	$2,$2
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_0014c398
	addiu	$2,$0,1
	b	loc_0014c398
	move	$2,$0
loc_0014c374:
	jal	0x00194508
	addiu	$5,$29,0x10
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_0014c398
	addiu	$2,$0,1
	move	$2,$0
loc_0014c398:
	andi	$2,$2,0xff
	.word	0x10400010
	ori	$3,$0,0x8000
	ld	$4,0x480($16)
	lw	$2,0x2d4($16)
	or	$4,$4,$3
	andi	$2,$2,0x10
	.word	0x1040000a
	sd	$4,0x480($16)
	lui	$2,1
	ld	$5,0x470($16)
	or	$2,$4,$2
	.word	0x10000006
	sd	$2,0x480($16)
	ori	$3,$0,0x8000
	or	$2,$2,$3
	.set	macro
	.set	reorder
	.end	boyAI_sub_14C338
