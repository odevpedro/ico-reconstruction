	.text
	.p2align 3
	.globl	boyAI_sub_15DCAC
	.ent	boyAI_sub_15DCAC
boyAI_sub_15DCAC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mtc1	$1,$f12
	lui	$1,0x42b4
	mtc1	$1,$f13
	beqz	$2,loc_0015dccc
	move	$5,$16
	lw	$4,0($29)
	jal	0x00106b80
	nop	
loc_0015dccc:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	.word	0x1000fff1
	lw	$2,0x20($29)
	addiu	$29,$29,-0x60
	addiu	$5,$0,1
	sw	$4,0($29)
	sd	$31,0x50($29)
	lw	$3,0($29)
	sd	$17,0x40($29)
	sd	$16,0x30($29)
	sw	$0,0x20($29)
	lw	$4,0($29)
	lw	$2,0xc($4)
	bne	$2,$5,loc_0015dd30
	lw	$17,0x164($3)
	lw	$4,0($29)
	addiu	$5,$29,0x20
	jal	0x00154218
	addiu	$6,$29,0x10
	.word	0x1000000f
	nop	
loc_0015dd30:
	lw	$4,-0x6e0c($28)
	jal	0x0015eff8
	addiu	$17,$17,0x110
	lw	$4,0($29)
	jal	0x0015eff8
	move	$16,$2
	move	$5,$16
	move	$6,$2
	.set	macro
	.set	reorder
	.end	boyAI_sub_15DCAC
