	.text
	.p2align 3
	.globl	girlBrain_sub_16B5DC
	.ent	girlBrain_sub_16B5DC
girlBrain_sub_16B5DC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	.word	0x46000834
	nop	
	bc1t	loc_0016b5f0
	addiu	$2,$0,1
	move	$2,$0
loc_0016b5f0:
	andi	$2,$2,0xff
	.word	0x1440004b
	addiu	$4,$23,0x2ac0
	addiu	$2,$23,0x2ac0
	addiu	$4,$29,0x20
	addiu	$20,$2,0x5820
	addiu	$19,$2,0x57f0
	lw	$18,0x3230($2)
	addiu	$16,$2,0x3240
	move	$5,$20
	jal	0x00243ae8
	move	$6,$19
	move	$17,$0
	sw	$0,0x24($29)
	addiu	$4,$29,0x30
	jal	0x001940b0
	addiu	$5,$29,0x20
	lwc1	$f0,0x20($29)
	lwc1	$f1,0x24($29)
	mul.s	$f0,$f0,$f0
	lwc1	$f12,0x28($29)
	mul.s	$f1,$f1,$f1
	mul.s	$f12,$f12,$f12
	add.s	$f0,$f0,$f1
	jal	0x00105fe0
	add.s	$f12,$f0,$f12
	.word	0x1a40002c
	mov.s	$f20,$f0
	mtc1	$0,$f22
	lwc1	$f21,-0x7b9c($28)
	lwc1	$f1,4($20)
	lwc1	$f0,0x14($16)
	sub.s	$f0,$f0,$f1
	cvt.w.s	$f1,$f0
	mfc1	$2,$f1
	nop	
	bltzl	$2,loc_0016b688
	negu	$2,$2
loc_0016b688:
	slti	$2,$2,0xc9
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16B5DC
