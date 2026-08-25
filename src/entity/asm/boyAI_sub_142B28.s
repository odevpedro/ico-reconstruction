	.text
	.p2align 3
	.globl	boyAI_sub_142B28
	.ent	boyAI_sub_142B28
boyAI_sub_142B28:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lui	$1,0x3f80
	mtc1	$1,$f1
	mtc1	$4,$f0
	cvt.s.w	$f0,$f0
	mul.s	$f0,$f0,$f2
	b	loc_00142b4c
	add.s	$f5,$f0,$f1
	lui	$1,0x3f80
	mtc1	$1,$f5
loc_00142b4c:
	bltz	$5,loc_00142ba0
	move	$4,$5
	lui	$1,0x3f80
	mtc1	$1,$f4
	slti	$2,$4,0x5b
	bnez	$2,loc_00142b70
	lwc1	$f0,0x1c($17)
	addiu	$2,$0,0xb4
	subu	$4,$2,$4
loc_00142b70:
	mtc1	$4,$f2
	cvt.s.w	$f2,$f2
	lui	$1,0x42b4
	mtc1	$1,$f1
	sub.s	$f0,$f4,$f0
	neg.s	$f0,$f0
	nop	
	nop	
	div.s	$f0,$f0,$f1
	mul.s	$f0,$f0,$f2
	b	loc_00142c00
	add.s	$f3,$f0,$f4
loc_00142ba0:
	negu	$4,$5
	lui	$1,0x3f80
	mtc1	$1,$f3
	slti	$2,$4,0x5b
	bnez	$2,loc_00142bc0
	lwc1	$f0,0x1c($17)
	addiu	$2,$0,0xb4
	subu	$4,$2,$4
loc_00142bc0:
	mtc1	$4,$f2
	cvt.s.w	$f2,$f2
	lui	$1,0x42b4
	mtc1	$1,$f1
	sub.s	$f0,$f3,$f0
	neg.s	$f0,$f0
	nop	
	nop	
	div.s	$f0,$f0,$f1
	mul.s	$f0,$f0,$f2
	b	loc_00142c00
	add.s	$f4,$f0,$f3
	lui	$1,0x3f80
	mtc1	$1,$f3
	mov.s	$f5,$f3
	mov.s	$f4,$f3
loc_00142c00:
	lui	$1,0x4580
	mtc1	$1,$f0
	move	$4,$17
	mul.s	$f0,$f6,$f0
	mul.s	$f0,$f0,$f5
	cvt.w.s	$f1,$f0
	mfc1	$2,$f1
	mtc1	$2,$f1
	cvt.s.w	$f1,$f1
	mul.s	$f0,$f1,$f4
	mul.s	$f1,$f1,$f3
	cvt.w.s	$f2,$f0
	mfc1	$2,$f2
	cvt.w.s	$f0,$f1
	.set	macro
	.set	reorder
	.end	boyAI_sub_142B28
