	.text
	.p2align 3
	.globl	boyAI_sub_14C6E4
	.ent	boyAI_sub_14C6E4
boyAI_sub_14C6E4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lwc1	$f0,0xc($5)
	.word	0x46010032
	nop	
	bc1f	loc_0014c708
	lui	$4,0x56
	jal	0x001a6e28
	addiu	$4,$4,-0x7e28
	b	loc_0014c77c
	move	$2,$0
loc_0014c708:
	lui	$1,0xbf80
	mtc1	$1,$f12
	jal	0x00243b18
	addiu	$4,$29,0x10
	move	$4,$17
	jal	0x00194508
	addiu	$5,$29,0x10
	bgez	$2,loc_0014c758
	move	$4,$17
	jal	0x00194508
	addiu	$5,$29,0x10
	negu	$2,$2
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_0014c77c
	addiu	$2,$0,1
	b	loc_0014c77c
	move	$2,$0
loc_0014c758:
	jal	0x00194508
	addiu	$5,$29,0x10
	mtc1	$2,$f0
	cvt.s.w	$f0,$f0
	.word	0x46140034
	nop	
	bc1t	loc_0014c77c
	addiu	$2,$0,1
	move	$2,$0
loc_0014c77c:
	andi	$2,$2,0xff
	.word	0x10400006
	addiu	$3,$0,0x2000
	ld	$2,0x480($16)
	ld	$5,0x470($16)
	or	$2,$2,$3
	.word	0x10000002
	sd	$2,0x480($16)
	.set	macro
	.set	reorder
	.end	boyAI_sub_14C6E4
