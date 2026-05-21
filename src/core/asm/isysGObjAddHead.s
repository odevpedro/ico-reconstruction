	.text
	.p2align 3
	.globl	isysGObjAddHead
	.ent	isysGObjAddHead
isysGObjAddHead:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x50
	lw	$7,-0x4c4c($28)
	sd	$18,0x20($29)
	sd	$19,0x30($29)
	move	$18,$4
	sd	$17,0x10($29)
	move	$19,$6
	sd	$31,0x40($29)
	andi	$17,$5,0xff
	sd	$16,0($29)
	beqz	$7,loc_0013ea40
	move	$4,$0
	lw	$3,-0x4c50($28)
	lw	$2,0($3)
	beqz	$2,loc_0013ea40
	nop	
	move	$5,$7
	addiu	$4,$4,1
loc_0013ea28:
	sltu	$2,$4,$5
	beqz	$2,loc_0013ea40
	addiu	$3,$3,0x174
	lw	$2,0($3)
	bnel	$2,$0,loc_0013ea28
	addiu	$4,$4,1
loc_0013ea40:
	bne	$4,$7,loc_0013ea60
	addiu	$2,$0,0x174
	lui	$4,0x55
	move	$16,$0
	jal	0x001a6e28
	addiu	$4,$4,0x7a30
	b	loc_0013ea78
	nop	
loc_0013ea60:
	lw	$3,-0x4c50($28)
	mult	$2,$4,$2
	addu	$2,$2,$3
	sw	$0,0x164($2)
	move	$16,$2
	sw	$0,0x170($2)
loc_0013ea78:
	bnel	$16,$0,loc_0013ea94
	sw	$18,0x28($16)
	lui	$4,0x55
	jal	0x001a6e28
	addiu	$4,$4,0x7a30
	b	loc_0013eac8
	move	$2,$0
loc_0013ea94:
	move	$5,$17
	sw	$16,0($16)
	move	$6,$19
	jal	0x0013e0c0
	move	$4,$16
	addiu	$3,$0,-1
	sw	$0,0x15c($16)
	sw	$3,4($16)
	move	$2,$16
	sw	$3,8($16)
	sw	$0,0x2c($16)
	sw	$0,0x30($16)
	sw	$0,0x58($16)
loc_0013eac8:
	ld	$31,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x50
	nop	
	lw	$8,-0x4c4c($28)
	beqz	$8,loc_0013eb44
	move	$6,$0
	lw	$7,-0x4c50($28)
	addiu	$2,$0,0x174
	nop	
loc_0013eb00:
	mult	$3,$6,$2
	addu	$5,$3,$7
	lw	$3,0($5)
	beqz	$3,loc_0013eb34
	addiu	$2,$0,1
	lw	$3,4($5)
	bne	$3,$2,loc_0013eb38
	addiu	$6,$6,1
	lw	$2,8($5)
	bne	$2,$4,loc_0013eb3c
	sltu	$2,$6,$8
	jr	$31
	move	$2,$5
loc_0013eb34:
	addiu	$6,$6,1
loc_0013eb38:
	sltu	$2,$6,$8
loc_0013eb3c:
	bnez	$2,loc_0013eb00
	addiu	$2,$0,0x174
loc_0013eb44:
	jr	$31
	move	$2,$0
	nop	
	lw	$2,-0x6730($28)
	beqz	$2,loc_0013ebb8
	move	$5,$4
	lw	$2,-0x4c4c($28)
	addiu	$3,$0,0x174
	lw	$4,-0x4c50($28)
	mult	$2,$2,$3
	addiu	$3,$4,-0x174
	addiu	$2,$2,-0x174
	addu	$4,$4,$2
	beq	$3,$4,loc_0013eba8
	addiu	$6,$0,1
	addiu	$3,$3,0x174
	nop	
loc_0013eb88:
	lw	$2,4($3)
	bne	$2,$6,loc_0013eba0
	nop	
	lw	$2,0xc($3)
	beq	$2,$5,loc_0013ebb0
	nop	
loc_0013eba0:
	bne	$3,$4,loc_0013eb88
	addiu	$3,$3,0x174
loc_0013eba8:
	jr	$31
	move	$2,$0
loc_0013ebb0:
	jr	$31
	move	$2,$3
loc_0013ebb8:
	addiu	$2,$5,-1
	sltiu	$2,$2,0x43
	beqz	$2,loc_0013eba8
	lui	$2,0x6b
	sll	$3,$5,2
	addiu	$2,$2,-0x6c30
	addu	$3,$3,$2
	jr	$31
	lw	$2,0($3)
	nop	
	lw	$2,-0x6730($28)
	beqz	$2,loc_0013ec48
	move	$6,$4
	lw	$2,-0x4c4c($28)
	addiu	$3,$0,0x174
	lw	$4,-0x4c50($28)
	move	$5,$6
	mult	$2,$2,$3
	addiu	$2,$2,-0x174
	addu	$4,$4,$2
	beq	$6,$4,loc_0013ec38
	lw	$3,0xc($6)
	addiu	$6,$0,1
	addiu	$5,$5,0x174
loc_0013ec18:
	lw	$2,4($5)
	bne	$2,$6,loc_0013ec30
	nop	
	lw	$2,0xc($5)
	beq	$2,$3,loc_0013ec40
	nop	
loc_0013ec30:
	bne	$5,$4,loc_0013ec18
	addiu	$5,$5,0x174
loc_0013ec38:
	jr	$31
	move	$2,$0
loc_0013ec40:
	jr	$31
	move	$2,$5
loc_0013ec48:
	jr	$31
	lw	$2,0x3c($6)
	lw	$8,-0x4c4c($28)
	beqz	$8,loc_0013ec9c
	move	$6,$0
	lw	$7,-0x4c50($28)
	addiu	$2,$0,0x174
	nop	
loc_0013ec68:
	mult	$3,$6,$2
	addu	$5,$3,$7
	lw	$3,0($5)
	beqz	$3,loc_0013ec90
	addiu	$6,$6,1
	lw	$2,4($5)
	bne	$2,$4,loc_0013ec94
	sltu	$2,$6,$8
	jr	$31
	move	$2,$5
loc_0013ec90:
	sltu	$2,$6,$8
loc_0013ec94:
	bnez	$2,loc_0013ec68
	addiu	$2,$0,0x174
loc_0013ec9c:
	jr	$31
	move	$2,$0
	nop	
	lw	$2,-0x4c4c($28)
	addiu	$3,$0,0x174
	lw	$4,-0x4c50($28)
	mult	$2,$2,$3
	addiu	$3,$4,-0x174
	addiu	$2,$2,-0x174
	addu	$4,$4,$2
	beq	$3,$4,loc_0013ece4
	addiu	$3,$3,0x174
	nop	
loc_0013ecd0:
	lw	$2,0($3)
	bnez	$2,loc_0013ecec
	move	$2,$3
	bne	$3,$4,loc_0013ecd0
	addiu	$3,$3,0x174
loc_0013ece4:
	jr	$31
	move	$2,$0
loc_0013ecec:
	jr	$31
	nop	
	nop	
	lw	$2,-0x4c4c($28)
	addiu	$3,$0,0x174
	lw	$5,-0x4c50($28)
	mult	$2,$2,$3
	addiu	$2,$2,-0x174
	addu	$5,$5,$2
	beq	$4,$5,loc_0013ed2c
	addiu	$4,$4,0x174
loc_0013ed18:
	lw	$2,0($4)
	bnez	$2,loc_0013ed34
	move	$2,$4
	bne	$4,$5,loc_0013ed18
	addiu	$4,$4,0x174
loc_0013ed2c:
	jr	$31
	move	$2,$0
loc_0013ed34:
	jr	$31
	nop	
	nop	
	bnez	$5,loc_0013ed60
	lw	$3,-0x6724($28)
	addiu	$2,$0,1
	sllv	$2,$2,$4
	nor	$2,$0,$2
	and	$3,$3,$2
	jr	$31
	sw	$3,-0x6724($28)
loc_0013ed60:
	addiu	$2,$0,1
	sllv	$2,$2,$4
	or	$3,$3,$2
	jr	$31
	sw	$3,-0x6724($28)
	nop	
	bnez	$5,loc_0013ed98
	lw	$3,-0x6720($28)
	addiu	$2,$0,1
	sllv	$2,$2,$4
	nor	$2,$0,$2
	and	$3,$3,$2
	jr	$31
	sw	$3,-0x6720($28)
loc_0013ed98:
	addiu	$2,$0,1
	sllv	$2,$2,$4
	or	$3,$3,$2
	jr	$31
	sw	$3,-0x6720($28)
	nop	
	move	$5,$4
	bnel	$5,$0,loc_0013edc8
	lw	$3,0x38($5)
	lui	$4,0x55
	j	0x1a6e28
	addiu	$4,$4,0x7a58
loc_0013edc8:
	bnel	$3,$0,loc_0013ede4
	lw	$2,0x34($5)
	lw	$3,0x34($5)
	beql	$3,$0,loc_0013ee00
	lbu	$6,0x40($5)
	b	loc_0013edec
	nop	
loc_0013ede4:
	sw	$2,0x34($3)
	lw	$3,0x34($5)
loc_0013edec:
	beql	$3,$0,loc_0013ee00
	lbu	$6,0x40($5)
	lw	$2,0x38($5)
	sw	$2,0x38($3)
	lbu	$6,0x40($5)
loc_0013ee00:
	lui	$2,0x28
	addiu	$2,$2,0x1ab0
	sll	$3,$6,2
	addu	$3,$3,$2
	lw	$4,0($3)
	bne	$5,$4,loc_0013ee2c
	lui	$2,0x28
	lw	$2,0x34($5)
	sw	$2,0($3)
	lbu	$6,0x40($5)
	lui	$2,0x28
loc_0013ee2c:
	sll	$4,$6,2
	addiu	$2,$2,0x1ad0
	addu	$4,$4,$2
	lw	$3,0($4)
	bne	$5,$3,loc_0013ee4c
	nop	
	lw	$2,0x38($5)
	sw	$2,0($4)
loc_0013ee4c:
	jr	$31
	nop	
	nop	
	j	0x13edb0
	nop	
	addiu	$29,$29,-0x40
	sd	$17,0x10($29)
	move	$17,$4
	sd	$18,0x20($29)
	sd	$16,0($29)
	lui	$4,0x55
	andi	$16,$5,0xff
	move	$18,$6
	sd	$31,0x30($29)
	jal	0x001a6e28
	addiu	$4,$4,0x7a68
	lui	$2,0x28
	sll	$3,$16,2
	addiu	$2,$2,0x1ab0
	sb	$16,0x40($17)
	addu	$6,$3,$2
	sw	$18,0x44($17)
	lw	$5,0($6)
	bnel	$5,$0,loc_0013eef0
	lw	$2,0x44($5)
	lui	$2,0x28
	sw	$0,0x38($17)
	addiu	$2,$2,0x1ad0
	sw	$0,0x34($17)
	addu	$2,$3,$2
	lui	$4,0x55
	sw	$17,0($2)
	addiu	$4,$4,0x7a80
	sw	$17,0($6)
	move	$5,$0
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x1a6e28
	addiu	$29,$29,0x40
loc_0013eef0:
	sltu	$2,$18,$2
	beqz	$2,loc_0013ef30
	lui	$2,0x28
	sw	$5,0x34($17)
	lui	$4,0x55
	sw	$0,0x38($17)
	addiu	$4,$4,0x7a90
	sw	$17,0x38($5)
	sw	$17,0($6)
	lw	$5,0x34($17)
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x1a6e28
	addiu	$29,$29,0x40
loc_0013ef30:
	addiu	$2,$2,0x1ad0
	addu	$16,$3,$2
	lw	$3,0($16)
	lw	$2,0x44($3)
	sltu	$2,$18,$2
	bnel	$2,$0,loc_0013ef84
	move	$4,$5
	sw	$0,0x34($17)
	lui	$4,0x55
	sw	$3,0x38($17)
	addiu	$4,$4,0x7aa0
	sw	$17,0x34($3)
	sw	$17,0($16)
	lw	$5,0x34($17)
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x1a6e28
	addiu	$29,$29,0x40
loc_0013ef80:
	move	$4,$3
loc_0013ef84:
	lw	$3,0x34($4)
	lw	$2,0x44($3)
	sltu	$2,$18,$2
	nop	
	beqz	$2,loc_0013ef80
	nop	
	sw	$4,0x38($17)
	ld	$31,0x30($29)
	lw	$2,0x34($4)
	ld	$18,0x20($29)
	sw	$2,0x34($17)
	sw	$17,0x34($4)
	ld	$16,0($29)
	lw	$2,0x34($17)
	sw	$17,0x38($2)
	ld	$17,0x10($29)
	jr	$31
	addiu	$29,$29,0x40
	nop	
	andi	$5,$5,0xff
	lui	$2,0x28
	sll	$8,$5,2
	addiu	$2,$2,0x1ab0
	sb	$5,0x40($4)
	addu	$3,$8,$2
	sw	$6,0x44($4)
	lw	$7,0($3)
	bnel	$7,$0,loc_0013f018
	lw	$2,0x44($7)
	lui	$2,0x28
	sw	$4,0($3)
	addiu	$2,$2,0x1ad0
	sw	$0,0x38($4)
	addu	$2,$8,$2
	sw	$0,0x34($4)
	jr	$31
	sw	$4,0($2)
loc_0013f018:
	sltu	$2,$2,$6
	bnez	$2,loc_0013f038
	lui	$2,0x28
	sw	$0,0x38($4)
	sw	$7,0x34($4)
	sw	$4,0($3)
	jr	$31
	sw	$4,0x38($7)
loc_0013f038:
	addiu	$2,$2,0x1ad0
	addu	$5,$8,$2
	lw	$3,0($5)
	lw	$2,0x44($3)
	sltu	$2,$2,$6
	beql	$2,$0,loc_0013f06c
	move	$5,$7
	sw	$3,0x38($4)
	sw	$0,0x34($4)
	sw	$4,0($5)
	jr	$31
	sw	$4,0x34($3)
loc_0013f068:
	move	$5,$3
loc_0013f06c:
	lw	$3,0x34($5)
	lw	$2,0x44($3)
	sltu	$2,$2,$6
	nop	
	bnez	$2,loc_0013f068
	nop	
	sw	$5,0x38($4)
	lw	$2,0x34($5)
	sw	$2,0x34($4)
	sw	$4,0x34($5)
	lw	$2,0x34($4)
	jr	$31
	sw	$4,0x38($2)
	addiu	$29,$29,-0x40
	sd	$16,0($29)
	move	$16,$4
	sd	$18,0x20($29)
	sd	$17,0x10($29)
	move	$18,$6
	sd	$31,0x30($29)
	jal	0x0013edb0
	andi	$17,$5,0xff
	move	$4,$16
	move	$5,$17
	move	$6,$18
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x13ee60
	addiu	$29,$29,0x40
	addiu	$29,$29,-0x40
	sd	$16,0($29)
	move	$16,$4
	sd	$18,0x20($29)
	sd	$17,0x10($29)
	move	$18,$6
	sd	$31,0x30($29)
	jal	0x0013edb0
	andi	$17,$5,0xff
	move	$4,$16
	move	$5,$17
	move	$6,$18
	ld	$31,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x13efd0
	addiu	$29,$29,0x40
	.set	macro
	.set	reorder
	.end	isysGObjAddHead
