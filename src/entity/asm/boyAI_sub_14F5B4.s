	.text
	.p2align 3
	.globl	boyAI_sub_14F5B4
	.ent	boyAI_sub_14F5B4
boyAI_sub_14F5B4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	jal	0x0015bcc8
	addiu	$5,$0,0x139
	.word	0x100001cf
	nop	
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d74($28)
	.word	0x46010034
	bc1f	loc_0014f604
	nop	
	lw	$2,0x330($18)
	addiu	$2,$2,-0x2e
	sltiu	$2,$2,0x59
	beqz	$2,loc_0014f604
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x138
	lwc1	$f1,0x33c($18)
	lwc1	$f0,-0x7d70($28)
	.word	0x46010034
loc_0014f604:
	.word	0x450001bd
	nop	
	lw	$3,0x330($18)
	slti	$2,$3,-0x86
	.word	0x144001b9
	slti	$2,$3,-0x2d
	.word	0x104001b7
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x139
	.word	0x100001b2
	nop	
	lw	$2,0x2d4($18)
	andi	$2,$2,0x40
	.word	0x104002d2
	nop	
	.word	0x1000024f
	nop	
	lw	$3,0x2d4($18)
	andi	$2,$3,0x10
	.word	0x14400006
	nop	
	lw	$2,0x32c($18)
	addiu	$2,$2,-0x80
	slti	$2,$2,-0x64
	.word	0x10400006
	.set	macro
	.set	reorder
	.end	boyAI_sub_14F5B4
