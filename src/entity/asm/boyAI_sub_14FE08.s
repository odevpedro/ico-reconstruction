	.text
	.p2align 3
	.globl	boyAI_sub_14FE08
	.ent	boyAI_sub_14FE08
boyAI_sub_14FE08:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lwc1	$f0,8($2)
	lw	$2,0x2d0($18)
	swc1	$f0,8($17)
	andi	$2,$2,0x20
	beqz	$2,loc_0014fe30
	swc1	$f20,-0x6658($28)
	jal	0x00190318
	lw	$4,0x180($18)
	b	loc_0014fe3c
	lw	$3,0x2d4($18)
loc_0014fe30:
	jal	0x00190330
	lw	$4,0x180($18)
	lw	$3,0x2d4($18)
loc_0014fe3c:
	andi	$2,$3,0x10
	beqz	$2,loc_0014fe5c
	andi	$2,$3,0x40
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb1
	lw	$3,0x2d4($18)
	andi	$2,$3,0x40
loc_0014fe5c:
	.word	0x104000cb
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x129
	.word	0x100000c6
	nop	
	lw	$3,0x32c($18)
	addiu	$2,$3,-0x80
	slti	$2,$2,0x65
	bnez	$2,loc_0014fea0
	addiu	$2,$3,-0x80
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xd2
	lw	$3,0x32c($18)
	addiu	$2,$3,-0x80
loc_0014fea0:
	slti	$2,$2,-0x64
	beql	$2,$0,loc_0014febc
	lw	$2,0x2d0($18)
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x93
	lw	$2,0x2d0($18)
loc_0014febc:
	andi	$2,$2,0x20
	.set	macro
	.set	reorder
	.end	boyAI_sub_14FE08
