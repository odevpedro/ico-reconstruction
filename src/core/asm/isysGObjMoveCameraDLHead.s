	.text
	.p2align 3
	.globl	isysGObjMoveCameraDLHead
	.ent	isysGObjMoveCameraDLHead
isysGObjMoveCameraDLHead:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x30
	sd	$16,0($29)
	sd	$17,0x10($29)
	move	$16,$4
	sd	$31,0x20($29)
	jal	0x001fbfc8
	move	$17,$5
	lw	$4,-0x671c($28)
	bnez	$4,0f
	sw	$17,0x44($16)
	sw	$16,-0x6718($28)
	sw	$0,0x38($16)
	sw	$0,0x34($16)
	b	4f
	sw	$16,-0x671c($28)
0:
	lw	$2,0x44($4)
	sltu	$2,$2,$17
	bnez	$2,1f
	lw	$3,-0x6718($28)
	sw	$0,0x38($16)
	sw	$4,0x34($16)
	sw	$16,0x38($4)
	b	4f
	sw	$16,-0x671c($28)
1:
	lw	$2,0x44($3)
	sltu	$2,$2,$17
	beql	$2,$0,3f
	lw	$3,0x34($4)
	sw	$3,0x38($16)
	sw	$0,0x34($16)
	sw	$16,0x34($3)
	b	4f
	sw	$16,-0x6718($28)
	nop	
2:
	move	$4,$3
	lw	$3,0x34($4)
3:
	lw	$2,0x44($3)
	sltu	$2,$2,$17
	nop	
	bnez	$2,2b
	nop	
	sw	$4,0x38($16)
	lw	$2,0x34($4)
	sw	$2,0x34($16)
	sw	$16,0x34($4)
	lw	$2,0x34($16)
	sw	$16,0x38($2)
4:
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	isysGObjMoveCameraDLHead
