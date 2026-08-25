	.text
	.p2align 3
	.globl	boyAI_sub_146B54
	.ent	boyAI_sub_146B54
boyAI_sub_146B54:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	dsrl32	$2,$3,0x1f
	beqz	$2,loc_00146b8c
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0x1f
	beqz	$2,loc_00146b8c
	move	$4,$21
	lw	$2,0x164($21)
	addiu	$5,$0,0xf9
	move	$6,$21
	lw	$7,0x678($2)
	jal	0x00194ff8
	addiu	$7,$7,0x7a0
	ld	$3,0x468($20)
loc_00146b8c:
	dsrl32	$2,$3,0x17
	andi	$2,$2,1
	beqz	$2,loc_00146bc8
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0x17
	andi	$2,$2,1
	beqz	$2,loc_00146bc8
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0xa1
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0xa0
	ld	$3,0x468($20)
loc_00146bc8:
	dsrl32	$2,$3,0x18
	andi	$2,$2,1
	beql	$2,$0,loc_00146bfc
	ld	$4,0x470($20)
	lbu	$2,0x47f($20)
	dsll32	$2,$2,0
	dsra32	$2,$2,0
	andi	$2,$2,1
	beqz	$2,loc_00146bf8
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0x9a
loc_00146bf8:
	ld	$4,0x470($20)
loc_00146bfc:
	dsll	$2,$4,0x1c
	dsra32	$2,$2,0
	andi	$2,$2,1
	.word	0x5040000f
	lw	$3,0x30($20)
	ld	$2,0x480($20)
	.set	macro
	.set	reorder
	.end	boyAI_sub_146B54
