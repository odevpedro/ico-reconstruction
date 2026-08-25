	.text
	.p2align 3
	.globl	boyAI_sub_157890
	.ent	boyAI_sub_157890
boyAI_sub_157890:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sd	$31,0x1b0($29)
	sd	$17,0x140($29)
	lw	$2,0($29)
	sd	$18,0x150($29)
	lw	$18,0x164($2)
	mtc1	$0,$f23
	lw	$17,0x600($18)
	lwc1	$f22,-0x7cb4($28)
	lui	$1,0x4700
	mtc1	$1,$f21
	jal	0x001c2e28
	move	$6,$17
	lwc1	$f20,-0x7cb0($28)
	lui	$2,0x56
	lui	$4,0x56
	addiu	$3,$2,-0x7760
	ld	$6,-0x7760($2)
	ld	$7,8($3)
	addiu	$5,$4,-0x7750
	ld	$3,8($5)
	addiu	$22,$29,0x40
	ld	$2,-0x7750($4)
	addiu	$21,$29,0x10
	sd	$6,0x50($29)
	sd	$7,0x58($29)
	sd	$2,0x60($29)
	sd	$3,0x68($29)
	nop	
	jal	0x00105278
	nop	
	jal	0x00243bd8
	move	$4,$2
	lwc1	$f13,0x24($29)
	lwc1	$f12,0x20($29)
	.set	macro
	.set	reorder
	.end	boyAI_sub_157890
