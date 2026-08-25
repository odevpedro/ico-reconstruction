	.text
	.p2align 3
	.globl	boyAI_sub_15E58C
	.ent	boyAI_sub_15E58C
boyAI_sub_15E58C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
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
	.word	0x1000ffe3
	lw	$2,0x3c($16)
	addiu	$29,$29,-0x50
	sw	$4,0($29)
	sd	$17,0x30($29)
	sd	$16,0x20($29)
	sd	$31,0x40($29)
	addiu	$16,$29,0x10
	lw	$2,0($29)
	lw	$3,0x164($2)
	addiu	$17,$3,0x5b0
	nop	
loc_0015e5f0:
	lw	$5,0($29)
	jal	0x00104748
	move	$4,$16
	move	$4,$17
	jal	0x00194508
	move	$5,$16
	bltzl	$2,loc_0015e610
	negu	$2,$2
loc_0015e610:
	slti	$2,$2,0xf
	beqz	$2,loc_0015e628
	addiu	$5,$0,0xe4
	lw	$4,0($29)
	jal	0x0015bcc8
	nop	
loc_0015e628:
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015e5f0
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	sd	$31,0x10($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15E58C
