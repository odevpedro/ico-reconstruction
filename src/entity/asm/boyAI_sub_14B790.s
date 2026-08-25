	.text
	.p2align 3
	.globl	boyAI_sub_14B790
	.ent	boyAI_sub_14B790
boyAI_sub_14B790:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	sw	$0,0x170($16)
	ld	$31,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x20
	nop	
	addiu	$29,$29,-0x50
	sd	$16,0x20($29)
	move	$16,$4
	sd	$17,0x30($29)
	move	$17,$5
	sd	$31,0x40($29)
	jal	0x001ae830
	addiu	$4,$29,0x10
	lw	$5,-0x6f60($28)
	addiu	$7,$29,0x10
	move	$6,$2
	jal	0x0019cbb8
	move	$4,$29
	move	$4,$16
	move	$6,$17
	jal	0x00194808
	move	$5,$29
	ld	$31,0x40($29)
	ld	$17,0x30($29)
	ld	$16,0x20($29)
	jr	$31
	addiu	$29,$29,0x50
	lw	$4,0x164($4)
	addiu	$3,$0,0x35
	lw	$2,0x30($4)
	beq	$2,$3,loc_0014b82c
	nop	
	lw	$2,0x678($4)
	lw	$3,0x394($2)
	.word	0x10600005
	lw	$2,-0x633c($28)
	.word	0x10400003
	nop	
loc_0014b82c:
	jr	$31
	.set	macro
	.set	reorder
	.end	boyAI_sub_14B790
