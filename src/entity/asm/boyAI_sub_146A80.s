	.text
	.p2align 3
	.globl	boyAI_sub_146A80
	.ent	boyAI_sub_146A80
boyAI_sub_146A80:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	nop	
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
	.word	0x1040000d
	nop	
	ld	$2,0x478($20)
	dsrl32	$2,$2,0x1d
	andi	$2,$2,1
	.set	macro
	.set	reorder
	.end	boyAI_sub_146A80
