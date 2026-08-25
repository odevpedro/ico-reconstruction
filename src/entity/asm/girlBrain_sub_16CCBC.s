	.text
	.p2align 3
	.globl	girlBrain_sub_16CCBC
	.ent	girlBrain_sub_16CCBC
girlBrain_sub_16CCBC:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	mtc1	$1,$f1
	addiu	$2,$3,-0x5770
	lwc1	$f0,0x14($2)
	.word	0x46000834
	nop	
	bc1f	loc_0016ccdc
	mov.s	$f22,$f20
	lwc1	$f22,-0x7b80($28)
loc_0016ccdc:
	lw	$2,0($29)
	addiu	$5,$0,-0x21
	move	$7,$0
	lw	$3,0x15c($2)
	addiu	$4,$0,7
	swc1	$f22,0x45c($3)
	ld	$2,0x20($20)
	and	$6,$2,$5
	.set	macro
	.set	reorder
	.end	girlBrain_sub_16CCBC
