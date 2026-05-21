	.text
	.p2align 3
	.globl	isysGObjDlInit
	.ent	isysGObjDlInit
isysGObjDlInit:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	lui	$3,0x28
	lui	$2,0x28
	addiu	$2,$2,0x1ad0
	addiu	$3,$3,0x1ab0
	addiu	$4,$0,7
	nop
0:
	sw	$0,0($3)
	addiu	$4,$4,-1
	sw	$0,0($2)
	addiu	$3,$3,4
	addiu	$2,$2,4
	bgez	$4,0b
	nop
	jr	$31
	.set	macro
	.set	reorder
	.end	isysGObjDlInit
