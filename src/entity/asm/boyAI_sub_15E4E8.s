	.text
	.p2align 3
	.globl	boyAI_sub_15E4E8
	.ent	boyAI_sub_15E4E8
boyAI_sub_15E4E8:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sw	$4,0($29)
	nop	
loc_0015e4f0:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015e4f0
	nop	
	nop	
	addiu	$29,$29,-0x80
	sw	$4,0($29)
	sd	$20,0x60($29)
	sd	$19,0x50($29)
	addiu	$20,$0,0xed
	sd	$18,0x40($29)
	sd	$17,0x30($29)
	addiu	$18,$29,0x10
	sd	$31,0x70($29)
	lw	$2,0($29)
	sd	$16,0x20($29)
	lw	$16,0x164($2)
	lw	$2,0x3c($16)
	addiu	$19,$16,0x610
	move	$17,$2
	nop	
loc_0015e550:
	move	$5,$17
	beq	$2,$20,loc_0015e564
	move	$6,$19
	b	loc_0015e574
	move	$17,$2
loc_0015e564:
	lw	$4,0($29)
	jal	0x001e29e8
	nop	
	sw	$2,0x120($16)
loc_0015e574:
	lw	$5,0($29)
	jal	0x00104748
	move	$4,$18
	lw	$3,0($29)
	move	$5,$18
	lw	$2,0x164($3)
	lw	$4,0x678($2)
	jal	0x00194508
	addiu	$4,$4,0x3d0
	bltzl	$2,loc_0015e5a0
	negu	$2,$2
loc_0015e5a0:
	slti	$2,$2,0xf
	beqz	$2,loc_0015e5b8
	addiu	$5,$0,0xe5
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
loc_0015e5b8:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015e550
	lw	$2,0x3c($16)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15E4E8
