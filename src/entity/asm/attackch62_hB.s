	.text
	.p2align 3
	.globl	attackch62_hB
	.ent	attackch62_hB
attackch62_hB:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	lw	$3,0x15c($4)
	lw	$9,0x800($3)
	lw	$2,0($9)
	blez	$2,loc_001bbe34
	move	$8,$0
	lw	$10,0xc($9)
	sll	$5,$8,3
	addiu	$7,$0,1
loc_001bbdf8:
	addu	$5,$5,$10
	addiu	$8,$8,1
	lw	$6,0($5)
	lw	$2,0x15c($6)
	lw	$3,0x800($2)
	lw	$4,4($3)
	sw	$4,4($5)
	lw	$2,0x15c($6)
	lw	$3,0x800($2)
	sw	$0,4($3)
	sw	$7,0x16c($6)
	lw	$2,0($9)
	slt	$2,$8,$2
	bnez	$2,loc_001bbdf8
	sll	$5,$8,3
loc_001bbe34:
	lw	$2,4($9)
	sw	$0,4($9)
	jr	$31
	sw	$2,8($9)
	.set	macro
	.set	reorder
	.end	attackch62_hB
