	.text
	.p2align 3
	.globl	boyAI_sub_150BCC
	.ent	boyAI_sub_150BCC
boyAI_sub_150BCC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bnez	$2,loc_00150c14
	nop	
	lw	$3,0x32c($20)
	addiu	$2,$3,-0x80
	slti	$2,$2,-0x64
	beqz	$2,loc_00150bfc
	addiu	$2,$3,-0x80
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x134
	lw	$3,0x32c($20)
	addiu	$2,$3,-0x80
loc_00150bfc:
	slti	$2,$2,0x65
	bnez	$2,loc_00150c14
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x135
loc_00150c14:
	lw	$4,0($29)
	addiu	$5,$0,0x74
	lw	$2,0x15c($4)
	lw	$3,0x4a0($2)
	.word	0x14650011
	nop	
	lw	$3,0x328($20)
	addiu	$2,$3,-0x80
	slti	$2,$2,-0x64
	beqz	$2,loc_00150c54
	addiu	$2,$3,-0x80
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x95
	lw	$3,0x328($20)
	addiu	$2,$3,-0x80
loc_00150c54:
	slti	$2,$2,0x65
	.word	0x14400004
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	.set	macro
	.set	reorder
	.end	boyAI_sub_150BCC
