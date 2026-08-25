	.text
	.p2align 3
	.globl	boyAI_sub_14DB24
	.ent	boyAI_sub_14DB24
boyAI_sub_14DB24:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	andi	$3,$3,1
	bnez	$3,loc_0014db60
	sw	$2,0x120($18)
loc_0014db30:
	jal	0x00203aa0
	addiu	$4,$0,1
	lw	$3,0x120($18)
	lw	$2,0x5c($3)
	andi	$2,$2,1
	beqz	$2,loc_0014db30
	lw	$2,-0x6e08($28)
	b	loc_0014db68
	addiu	$5,$0,0x5b
	nop	
loc_0014db58:
	jal	0x00203aa0
	nop	
loc_0014db60:
	lw	$2,-0x6e08($28)
	addiu	$5,$0,0x5b
loc_0014db68:
	beqz	$2,loc_0014db78
	move	$4,$2
	jal	0x0013ff88
	lw	$6,-0x6714($28)
loc_0014db78:
	lw	$2,0xd0($18)
	andi	$2,$2,2
	beqz	$2,loc_0014db58
	addiu	$4,$0,1
	lw	$4,0($29)
	move	$6,$17
	jal	0x001e29e8
	addiu	$5,$0,0x5c
	lw	$3,0x5c($2)
	andi	$3,$3,1
	.word	0x1460000d
	sw	$2,0x120($18)
loc_0014dba8:
	jal	0x00203aa0
	addiu	$4,$0,1
	lw	$3,0x120($18)
	lw	$2,0x5c($3)
	andi	$2,$2,1
	beqz	$2,loc_0014dba8
	.set	macro
	.set	reorder
	.end	boyAI_sub_14DB24
