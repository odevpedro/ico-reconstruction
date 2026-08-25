	.text
	.p2align 3
	.globl	boyAI_sub_14B6A0
	.ent	boyAI_sub_14B6A0
boyAI_sub_14B6A0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mtc1	$1,$f1
	nop	
	nop	
	div.s	$f0,$f0,$f2
	mul.s	$f0,$f0,$f1
	cvt.w.s	$f1,$f0
	mfc1	$16,$f1
	jal	0x0010d530
	addu	$16,$16,$17
	sll	$16,$16,0xf
	addiu	$2,$0,0xb4
	div	$0,$16,$2
	move	$4,$18
	beql	$2,$0,loc_0014b6dc
	break	0,7
loc_0014b6dc:
	mflo	$16
	sll	$16,$16,0x10
	jal	0x0010e158
	sra	$5,$16,0x10
	ld	$31,0x40($29)
	ld	$18,0x30($29)
	ld	$17,0x20($29)
	ld	$16,0x10($29)
	jr	$31
	addiu	$29,$29,0x50
	nop	
	addiu	$29,$29,-0x40
	addiu	$6,$0,0x10
	sd	$17,0x20($29)
	sd	$16,0x10($29)
	move	$17,$5
	move	$16,$4
	move	$5,$0
	sd	$31,0x30($29)
	jal	0x002641d8
	move	$4,$29
	lui	$1,0x3f80
	.set	macro
	.set	reorder
	.end	boyAI_sub_14B6A0
