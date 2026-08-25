	.text
	.p2align 3
	.globl	boyAI_sub_157918
	.ent	boyAI_sub_157918
boyAI_sub_157918:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	add.s	$f13,$f13,$f23
	jal	0x00105308
	lwc1	$f14,0x28($29)
	mtc1	$16,$f0
	cvt.s.w	$f0,$f0
	addiu	$16,$16,1
	mul.s	$f0,$f0,$f22
	mul.s	$f0,$f0,$f21
	nop	
	nop	
	div.s	$f0,$f0,$f20
	cvt.w.s	$f1,$f0
	mfc1	$4,$f1
	sll	$4,$4,0x10
	jal	0x00104fc0
	sra	$4,$4,0x10
	jal	0x00105278
	nop	
	move	$5,$2
	addiu	$4,$29,0x70
	jal	0x002438b8
	addiu	$6,$29,0x50
	jal	0x00105278
	nop	
	move	$5,$2
	addiu	$4,$29,0x80
	jal	0x002438b8
	addiu	$6,$29,0x60
	jal	0x001683c8
	addiu	$4,$29,0x70
	lw	$2,0xf8($29)
	beqz	$2,loc_001579bc
	slti	$3,$16,4
	ldl	$2,0xf7($29)
	ldr	$2,0xf0($29)
	sdl	$2,0x17($29)
	sdr	$2,0x10($29)
	addiu	$4,$0,1
	lw	$2,0xf8($29)
	.word	0x10000004
	sw	$2,0x18($29)
loc_001579bc:
	.word	0x1460ffd0
	nop	
	move	$4,$0
	.set	macro
	.set	reorder
	.end	boyAI_sub_157918
