	.text
	.p2align 3
	.globl	boyAI_sub_153E88
	.ent	boyAI_sub_153E88
boyAI_sub_153E88:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sltiu	$2,$2,0x68
	beqz	$2,loc_00153e9c
	move	$6,$0
	beql	$3,$0,loc_00153e9c
	addiu	$6,$0,1
loc_00153e9c:
	andi	$2,$6,0xff
	bnez	$2,loc_00153eb4
	nop	
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
loc_00153eb4:
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ffee
	lw	$4,-0x6e08($28)
	nop	
	addiu	$29,$29,-0x20
	sd	$31,0x10($29)
	sw	$4,0($29)
	lw	$2,-0x6e08($28)
loc_00153ed8:
	addiu	$5,$0,0x163
	beqz	$2,loc_00153eec
	move	$4,$2
	jal	0x0013ff88
	lw	$6,-0x6714($28)
loc_00153eec:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00153ed8
	lw	$2,-0x6e08($28)
	addiu	$29,$29,-0x20
	sd	$31,0x10($29)
	sw	$4,0($29)
	nop	
loc_00153f18:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x167
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00153f18
	.set	macro
	.set	reorder
	.end	boyAI_sub_153E88
