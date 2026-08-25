	.text
	.p2align 3
	.globl	eBrainGetStatus
	.ent	eBrainGetStatus
eBrainGetStatus:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$2,-0x4b38($28)
	bltz	$2,loc_00191d3c
	move	$6,$4
	bne	$6,$2,loc_00191d48
	lw	$5,-0x4b3c($28)
	jr	$31
	addiu	$2,$0,1
loc_00191d3c:
	sw	$6,-0x4b38($28)
	jr	$31
	addiu	$2,$0,1
loc_00191d48:
	lui	$4,0x6d
	addiu	$4,$4,0xb90
	move	$2,$0
	sll	$3,$5,2
	addiu	$5,$5,1
	addu	$3,$3,$4
	sw	$5,-0x4b3c($28)
	jr	$31
	sw	$6,0($3)
	nop	
	.set	macro
	.set	reorder
	.end	eBrainGetStatus
