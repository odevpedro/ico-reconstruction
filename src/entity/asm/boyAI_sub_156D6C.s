	.text
	.p2align 3
	.globl	boyAI_sub_156D6C
	.ent	boyAI_sub_156D6C
boyAI_sub_156D6C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	cvt.w.s	$f1,$f0
	mfc1	$21,$f1
	move	$4,$17
loc_00156d78:
	move	$5,$0
	addiu	$6,$0,0x10
	jal	0x002641d8
	addiu	$18,$18,1
	subu	$4,$21,$19
	jal	0x00194960
	swc1	$f22,0x18($29)
	addiu	$19,$19,5
	mtc1	$2,$f12
	cvt.s.w	$f12,$f12
	mul.s	$f12,$f12,$f21
	nop	
	nop	
	div.s	$f12,$f12,$f20
	jal	0x001945b8
	move	$4,$17
	lw	$4,0($29)
	jal	0x0015eff8
	nop	
	lui	$1,0x4348
	mtc1	$1,$f12
	move	$7,$0
	addiu	$8,$0,0xff
	move	$4,$2
	move	$5,$17
	jal	0x001873a8
	addiu	$6,$0,0xff
	lw	$4,0($29)
	jal	0x00106ab8
	move	$5,$17
	div	$0,$18,$20
	addiu	$4,$29,0x20
	beql	$20,$0,loc_00156e00
	break	0,7
loc_00156e00:
	mfhi	$2
	bnez	$2,loc_00156e30
	nop	
	lw	$16,0($29)
	jal	0x00156558
	move	$5,$16
	move	$4,$16
	jal	0x00106ab8
	addiu	$5,$29,0x20
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x13a
loc_00156e30:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00156d78
	move	$4,$17
	.set	macro
	.set	reorder
	.end	boyAI_sub_156D6C
