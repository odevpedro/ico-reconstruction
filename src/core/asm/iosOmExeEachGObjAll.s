	.text
	.p2align 3
	.globl	iosOmExeEachGObjAll
	.ent	iosOmExeEachGObjAll
iosOmExeEachGObjAll:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x60
	lui	$2,0x28
	sd	$20,0x40($29)
	move	$3,$0
	sd	$19,0x30($29)
	addiu	$20,$2,0x1a70
	sd	$18,0x20($29)
	move	$19,$4
	sd	$31,0x50($29)
	move	$18,$5
	sd	$17,0x10($29)
	sd	$16,0($29)
	sll	$2,$3,2
	nop	
0:
	addu	$2,$2,$20
	lw	$16,0($2)
	beqz	$16,2f
	addiu	$17,$3,1
	move	$4,$16
	nop	
1:
	jalr	$19
	move	$5,$18
	lw	$16,0x10($16)
	bnez	$16,1b
	move	$4,$16
	b	3f
	move	$3,$17
2:
	move	$3,$17
3:
	slti	$2,$3,8
	bnel	$2,$0,0b
	sll	$2,$3,2
	ld	$31,0x50($29)
	ld	$20,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	iosOmExeEachGObjAll
