	.text
	.p2align 3
	.globl	IsEnableCallEnemy
	.ent	IsEnableCallEnemy
IsEnableCallEnemy:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	addiu	$29,$29,-0x30
	sd	$16,0($29)
	move	$16,$4
	sd	$31,0x20($29)
	sd	$17,0x10($29)
	jal	0x0013eb50
	addiu	$4,$0,4
	move	$6,$2
	beqz	$6,loc_00193b5c
	lui	$17,0x2a
	lw	$5,8($6)
	nop	
loc_00193aa8:
	addiu	$2,$0,0x4c
	addiu	$3,$17,0x4c48
	lw	$8,0x164($6)
	mult	$2,$5,$2
	beqz	$16,loc_00193adc
	addu	$7,$2,$3
	lw	$3,8($16)
	addiu	$2,$0,0xd7e
	bne	$3,$2,loc_00193ae0
	addiu	$2,$0,0x4c
	addiu	$2,$0,0xd7d
	bne	$5,$2,loc_00193b48
	nop	
loc_00193adc:
	addiu	$2,$0,0x4c
loc_00193ae0:
	addiu	$4,$17,0x4c48
	mult	$2,$5,$2
	addu	$2,$2,$4
	lhu	$3,0x44($2)
	beql	$3,$0,loc_00193b10
	ld	$2,0x18($8)
	beql	$16,$0,loc_00193b10
	ld	$2,0x18($8)
	lw	$2,8($16)
	bne	$3,$2,loc_00193b48
	nop	
	ld	$2,0x18($8)
loc_00193b10:
	dsrl32	$2,$2,2
	andi	$2,$2,1
	bnez	$2,loc_00193b48
	nop	
	lw	$2,0x48($7)
	srl	$2,$2,0x15
	andi	$2,$2,1
	bnez	$2,loc_00193b48
	addiu	$2,$0,-1
	lh	$3,0x42($7)
	beq	$3,$2,loc_00193b60
	move	$2,$6
	bgtz	$3,loc_00193b64
	ld	$31,0x20($29)
loc_00193b48:
	jal	0x0013ebe0
	move	$4,$6
	move	$6,$2
	bnel	$6,$0,loc_00193aa8
	lw	$5,8($6)
loc_00193b5c:
	move	$2,$0
loc_00193b60:
	ld	$31,0x20($29)
loc_00193b64:
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x30
	nop	
	.set	macro
	.set	reorder
	.end	IsEnableCallEnemy
