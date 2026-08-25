	.text
	.p2align 3
	.globl	boyAI_sub_14D1F0
	.ent	boyAI_sub_14D1F0
boyAI_sub_14D1F0:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$4,$18
	mtc1	$0,$f0
	.word	0x46000834
	bc1f	loc_0014d228
	nop	
	neg.s	$f1,$f1
	lui	$1,0x4316
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1t	loc_0014d240
	addiu	$16,$29,0x20
	b	loc_0014d28c
	move	$4,$18
loc_0014d228:
	lui	$1,0x4316
	mtc1	$1,$f0
	.word	0x46000834
	nop	
	bc1f	loc_0014d288
	addiu	$16,$29,0x20
loc_0014d240:
	move	$4,$18
	move	$5,$0
	addiu	$6,$0,2
	addiu	$7,$0,1
	move	$8,$17
	move	$9,$0
	jal	0x00149d60
	move	$10,$16
	move	$10,$16
	move	$4,$18
	addiu	$5,$0,1
	addiu	$6,$0,2
	addiu	$7,$0,1
	move	$8,$17
	jal	0x00149d60
	move	$9,$0
	.word	0x10000011
	nop	
loc_0014d288:
	move	$4,$18
loc_0014d28c:
	move	$5,$0
	addiu	$6,$0,2
	move	$7,$0
	move	$8,$0
	move	$9,$0
	jal	0x00149d60
	move	$10,$0
	move	$4,$18
	addiu	$5,$0,1
	addiu	$6,$0,2
	move	$7,$0
	move	$8,$0
	move	$9,$0
	jal	0x00149d60
	.set	macro
	.set	reorder
	.end	boyAI_sub_14D1F0
