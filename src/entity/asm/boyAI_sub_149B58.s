	.text
	.p2align 3
	.globl	boyAI_sub_149B58
	.ent	boyAI_sub_149B58
boyAI_sub_149B58:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mtc1	$0,$f1
	.word	0x46000834
	nop	
	.word	0x4501002b
	ld	$31,0x50($29)
	lui	$1,0xbf80
	mtc1	$1,$f20
	jal	0x0018cec0
	nop	
	move	$5,$2
	move	$4,$29
	jal	0x00243b18
	mov.s	$f12,$f20
	jal	0x0015eff8
	lw	$4,-0x6e08($28)
	move	$5,$2
	mov.s	$f12,$f20
	jal	0x00243b18
	move	$4,$17
	lui	$2,0x27
	addiu	$4,$0,0xa
	lw	$7,0x4ec0($2)
	addiu	$3,$0,0x3c
	addiu	$2,$2,0x4ec0
	lui	$1,0x4270
	mtc1	$1,$f2
	.word	0x00e43818
	lw	$6,4($2)
	lui	$2,0x29
	move	$5,$17
	lwc1	$f1,0x24d8($2)
	beql	$6,$0,loc_00149bdc
	break	0,7
loc_00149bdc:
	subu	$3,$3,$7
	div	$0,$3,$6
	mflo	$3
	mtc1	$3,$f0
	cvt.s.w	$f0,$f0
	mul.s	$f1,$f1,$f0
	nop	
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_149B58
