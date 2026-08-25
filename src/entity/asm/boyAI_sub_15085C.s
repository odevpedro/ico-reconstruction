	.text
	.p2align 3
	.globl	boyAI_sub_15085C
	.ent	boyAI_sub_15085C
boyAI_sub_15085C:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	move	$4,$18
	move	$16,$2
	jal	0x0015eff8
	move	$4,$17
	move	$4,$16
	jal	0x001943c8
	move	$5,$2
	.word	0x46140034
	.word	0x45000029
	nop	
	jal	0x0015eff8
	move	$4,$17
	move	$16,$2
	jal	0x0015eff8
	move	$4,$18
	move	$5,$16
	move	$6,$2
	jal	0x00243ae8
	addiu	$4,$29,0x20
	jal	0x0015ef88
	move	$4,$18
	move	$5,$2
	jal	0x00194508
	addiu	$4,$29,0x20
	bgez	$2,loc_001508e0
	nop	
	jal	0x0015ef88
	move	$4,$18
	move	$5,$2
	jal	0x00194508
	addiu	$4,$29,0x20
	.word	0x10000007
	negu	$3,$2
loc_001508e0:
	jal	0x0015ef88
	move	$4,$18
	move	$5,$2
	jal	0x00194508
	addiu	$4,$29,0x20
	.set	macro
	.set	reorder
	.end	boyAI_sub_15085C
