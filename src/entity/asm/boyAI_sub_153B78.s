	.text
	.p2align 3
	.globl	boyAI_sub_153B78
	.ent	boyAI_sub_153B78
boyAI_sub_153B78:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$4,0x678($5)
	lw	$6,0x678($2)
	lw	$5,0x7f8($7)
	addiu	$4,$4,0x800
	jal	0x001688e0
	addiu	$6,$6,0x7f0
	lw	$2,-0x6e08($28)
	nop	
loc_00153b98:
	beqz	$2,loc_00153bc0
	nop	
	jal	0x00182be8
	nop	
	lw	$2,-0x6e08($28)
	addiu	$5,$0,0x3a
	beqz	$2,loc_00153bc0
	move	$4,$2
	jal	0x0013ff88
	lw	$6,-0x6714($28)
loc_00153bc0:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0x117
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_00153b98
	lw	$2,-0x6e08($28)
	nop	
	addiu	$29,$29,-0x30
	sw	$4,0($29)
	sd	$31,0x20($29)
	lw	$5,0($29)
	sd	$16,0x10($29)
	lw	$2,0($29)
	lw	$16,0x164($5)
	lw	$3,0x164($2)
	lw	$4,0($29)
	lw	$5,0x678($3)
	jal	0x0015d348
	addiu	$5,$5,0x7f0
	lw	$2,0($29)
	lw	$5,0($29)
	lw	$3,0x164($2)
	lwc1	$f0,0x4b0($16)
	lw	$2,0x678($3)
	lw	$4,0x164($5)
	swc1	$f0,0x430($2)
	lw	$5,0x678($4)
	lw	$6,0($29)
	lwc1	$f0,0x4b4($16)
	lw	$3,0($29)
	swc1	$f0,0x434($5)
	lw	$2,0x164($6)
	lw	$4,0x164($3)
	lw	$5,0x678($2)
	lwc1	$f0,0x4b8($16)
	lw	$2,0x678($4)
	swc1	$f0,0x438($5)
	ldl	$3,0x627($16)
	ldr	$3,0x620($16)
	lw	$4,0x628($16)
	sdl	$3,0x447($2)
	.set	macro
	.set	reorder
	.end	boyAI_sub_153B78
