	.text
	.p2align 3
	.globl	boyAI_sub_146DD8
	.ent	boyAI_sub_146DD8
boyAI_sub_146DD8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	beqz	$2,loc_00146e08
	nop	
	ld	$2,0x480($20)
	dsll	$2,$2,3
	dsra32	$2,$2,0
	andi	$2,$2,1
	beqz	$2,loc_00146e08
	nop	
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0x72
	ld	$4,0x470($20)
loc_00146e08:
	dsll	$2,$4,1
	dsra32	$2,$2,0
	andi	$2,$2,1
	beqz	$2,loc_00146e44
	nop	
	ld	$2,0x480($20)
	dsll	$2,$2,1
	dsra32	$2,$2,0
	andi	$2,$2,1
	beqz	$2,loc_00146e44
	nop	
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0x73
	ld	$4,0x470($20)
loc_00146e44:
	dsrl32	$2,$4,1
	andi	$2,$2,1
	beqz	$2,loc_00146e78
	nop	
	ld	$2,0x480($20)
	dsrl32	$2,$2,1
	andi	$2,$2,1
	beqz	$2,loc_00146e78
	nop	
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0x7a
	ld	$4,0x470($20)
loc_00146e78:
	dsra32	$2,$4,0
	andi	$2,$2,1
	.word	0x1040000b
	nop	
	.set	macro
	.set	reorder
	.end	boyAI_sub_146DD8
