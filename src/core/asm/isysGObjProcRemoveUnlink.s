	.text
	.p2align 3
	.globl	isysGObjProcRemoveUnlink
	.ent	isysGObjProcRemoveUnlink
isysGObjProcRemoveUnlink:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	bnel	$4,$0,loc_0013f64c
	lw	$3,0xc($4)
	lui	$4,0x55
	j	0x1a6e28
	addiu	$4,$4,0x7b48
loc_0013f64c:
	bnel	$3,$0,loc_0013f668
	lw	$2,8($4)
	lw	$3,8($4)
	beql	$3,$0,loc_0013f684
	lw	$3,4($4)
	b	loc_0013f670
	nop	
loc_0013f668:
	sw	$2,8($3)
	lw	$3,8($4)
loc_0013f670:
	beql	$3,$0,loc_0013f684
	lw	$3,4($4)
	lw	$2,0xc($4)
	sw	$2,0xc($3)
	lw	$3,4($4)
loc_0013f684:
	lw	$2,0x2c($3)
	bne	$4,$2,loc_0013f698
	move	$5,$3
	lw	$2,8($4)
	sw	$2,0x2c($3)
loc_0013f698:
	lw	$2,0x30($5)
	bne	$4,$2,loc_0013f6ac
	nop	
	lw	$2,0xc($4)
	sw	$2,0x30($5)
loc_0013f6ac:
	jr	$31
	nop	
	nop	
	.set	macro
	.set	reorder
	.end	isysGObjProcRemoveUnlink
