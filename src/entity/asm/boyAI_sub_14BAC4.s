	.text
	.p2align 3
	.globl	boyAI_sub_14BAC4
	.ent	boyAI_sub_14BAC4
boyAI_sub_14BAC4:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$2,$0
	addiu	$3,$0,2
	addiu	$2,$0,1
	movz	$2,$3,$21
	ld	$31,0x130($29)
	ld	$21,0x120($29)
	ld	$20,0x110($29)
	ld	$19,0x100($29)
	ld	$18,0xf0($29)
	ld	$17,0xe0($29)
	ld	$16,0xd0($29)
	lwc1	$f20,0x140($29)
	jr	$31
	addiu	$29,$29,0x150
	nop	
	lw	$2,0x164($4)
	lui	$1,0x3f80
	mtc1	$1,$f0
	lw	$3,0x670($2)
	swc1	$f0,0x58($3)
	jr	$31
	sw	$0,0x54($3)
	nop	
	addiu	$29,$29,-0x10
	move	$6,$4
	sd	$31,0($29)
	lui	$1,0x3f80
	mtc1	$1,$f12
	lw	$2,0x164($6)
	lw	$7,0x670($2)
	lw	$3,0x54($7)
	sltiu	$2,$3,3
	.word	0x10400004
	.set	macro
	.set	reorder
	.end	boyAI_sub_14BAC4
