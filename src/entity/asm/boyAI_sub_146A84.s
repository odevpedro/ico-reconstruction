	.text
	.p2align 3
	.globl	boyAI_sub_146A84
	.ent	boyAI_sub_146A84
boyAI_sub_146A84:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	ld	$2,0x478($20)
	dsrl32	$2,$2,0x1b
	andi	$2,$2,1
	beqz	$2,loc_00146aa4
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0xc4
	ld	$3,0x468($20)
loc_00146aa4:
	dsrl32	$2,$3,0x1c
	andi	$2,$2,1
	beqz	$2,loc_00146ad4
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0x1c
	andi	$2,$2,1
	beqz	$2,loc_00146ad4
	move	$4,$21
	jal	0x0015bcc8
	addiu	$5,$0,0xc5
	ld	$3,0x468($20)
loc_00146ad4:
	dsrl32	$2,$3,0x1d
	andi	$2,$2,1
	beqz	$2,loc_00146b14
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0x1d
	andi	$2,$2,1
	beqz	$2,loc_00146b14
	move	$4,$21
	lw	$2,0x164($21)
	addiu	$5,$0,0xf7
	move	$6,$21
	lw	$7,0x678($2)
	jal	0x00194ff8
	addiu	$7,$7,0x740
	ld	$3,0x468($20)
loc_00146b14:
	dsrl32	$2,$3,0x1e
	andi	$2,$2,1
	beqz	$2,loc_00146b54
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0x1e
	andi	$2,$2,1
	beqz	$2,loc_00146b54
	move	$4,$21
	lw	$2,0x164($21)
	addiu	$5,$0,0xf8
	move	$6,$21
	lw	$7,0x678($2)
	jal	0x00194ff8
	addiu	$7,$7,0x770
	ld	$3,0x468($20)
loc_00146b54:
	dsrl32	$2,$3,0x1f
	.set	macro
	.set	reorder
	.end	boyAI_sub_146A84
