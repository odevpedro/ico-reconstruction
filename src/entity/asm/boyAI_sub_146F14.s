	.text
	.p2align 3
	.globl	boyAI_sub_146F14
	.ent	boyAI_sub_146F14
boyAI_sub_146F14:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	ld	$4,0x470($20)
	dsrl32	$2,$4,4
	andi	$2,$2,1
	beqz	$2,loc_00146f4c
	nop	
	ld	$2,0x480($20)
	dsrl32	$2,$2,4
	andi	$2,$2,1
	beqz	$2,loc_00146f4c
	nop	
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0x74
	ld	$4,0x470($20)
loc_00146f4c:
	dsrl32	$2,$4,5
	andi	$2,$2,1
	beqz	$2,loc_00146f8c
	nop	
	ld	$2,0x480($20)
	dsrl32	$2,$2,5
	andi	$2,$2,1
	beqz	$2,loc_00146f8c
	nop	
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0x75
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0x76
	ld	$4,0x470($20)
loc_00146f8c:
	dsll	$2,$4,6
	dsra32	$2,$2,0
	andi	$2,$2,1
	beqz	$2,loc_00146fc8
	nop	
	ld	$2,0x480($20)
	dsll	$2,$2,6
	dsra32	$2,$2,0
	andi	$2,$2,1
	beqz	$2,loc_00146fc8
	nop	
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0x6f
	ld	$4,0x470($20)
loc_00146fc8:
	dsll	$2,$4,5
	.set	macro
	.set	reorder
	.end	boyAI_sub_146F14
