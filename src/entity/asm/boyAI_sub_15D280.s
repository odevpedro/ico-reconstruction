	.text
	.p2align 3
	.globl	boyAI_sub_15D280
	.ent	boyAI_sub_15D280
boyAI_sub_15D280:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$4,0xc($3)
	addu	$16,$16,$4
	lwc1	$f0,0x38($16)
	swc1	$f0,0x18($29)
	lw	$4,0x15c($8)
	lw	$3,0xc($4)
	addu	$3,$2,$3
	lwc1	$f0,0x30($3)
	swc1	$f0,0x20($29)
	lw	$4,0x15c($8)
	lw	$3,0xc($4)
	addu	$3,$2,$3
	lwc1	$f0,0x34($3)
	swc1	$f0,0x24($29)
	lw	$3,0x15c($8)
	lw	$4,0xc($3)
	addu	$2,$2,$4
	lwc1	$f0,0x38($2)
	swc1	$f0,0x28($29)
	lw	$3,0x15c($7)
	lw	$2,0x4a0($3)
	mult	$3,$2,$9
	addu	$2,$3,$18
	lw	$3,0x188($2)
	srl	$3,$3,0x1b
	andi	$3,$3,1
	beqz	$3,loc_0015d304
	move	$6,$17
	lw	$4,0($29)
	jal	0x00102c10
	nop	
	b	loc_0015d318
	nop	
loc_0015d304:
	lw	$4,0($29)
	addiu	$5,$0,6
	lwc1	$f12,-0x7c28($28)
	jal	0x00102c10
	move	$6,$17
loc_0015d318:
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000ffbf
	lw	$4,-0x6e08($28)
	lw	$2,-0x6dfc($28)
	.set	macro
	.set	reorder
	.end	boyAI_sub_15D280
