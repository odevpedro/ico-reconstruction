	.text
	.p2align 3
	.globl	iosOmCreateDL
	.ent	iosOmCreateDL
iosOmCreateDL:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x50
	sd	$18,0x20($29)
	lw	$18,-0x671c($28)
	sd	$31,0x40($29)
	sd	$19,0x30($29)
	sd	$17,0x10($29)
	beqz	$18,7f
	sd	$16,0($29)
	lui	$2,0x28
	addiu	$19,$2,0x1ab0
	lw	$3,-0x6724($28)
	andi	$2,$3,1
0:
	beqz	$2,1f
	move	$4,$0
	lw	$2,0x48($18)
	beql	$2,$0,2f
	srav	$2,$3,$4
	jalr	$2
	move	$4,$18
	lw	$3,-0x6724($28)
	move	$4,$0
	nop	
1:
	srav	$2,$3,$4
2:
	andi	$2,$2,1
	beqz	$2,5f
	addiu	$17,$4,1
	lw	$2,0x4c($18)
	srav	$2,$2,$4
	andi	$2,$2,1
	beqz	$2,5f
	sll	$2,$4,2
	addu	$2,$2,$19
	lw	$16,0($2)
	beqz	$16,6f
	move	$4,$17
	lw	$2,0x16c($16)
3:
	beql	$2,$0,4f
	lw	$16,0x34($16)
	lw	$3,0x50($18)
	lw	$2,0x50($16)
	and	$3,$3,$2
	beql	$3,$0,4f
	lw	$16,0x34($16)
	lw	$2,0x48($16)
	beql	$2,$0,4f
	lw	$16,0x34($16)
	jalr	$2
	move	$4,$16
	lw	$16,0x34($16)
4:
	bnel	$16,$0,3b
	lw	$2,0x16c($16)
	b	6f
	move	$4,$17
5:
	move	$4,$17
6:
	slti	$2,$4,0x20
	bnez	$2,1b
	lw	$3,-0x6724($28)
	lw	$18,0x34($18)
	bnez	$18,0b
	andi	$2,$3,1
7:
	ld	$31,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	.set	macro
	.set	reorder
	.end	iosOmCreateDL
