	.text
	.p2align 3
	.globl	type24_hA
	.ent	type24_hA
type24_hA:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x30
	sd	$16,0($29)
	sd	$31,0x20($29)
	move	$16,$4
	sd	$17,0x10($29)
	lw	$2,0x15c($16)
	jal	0x0010ecb8
	lw	$17,0x800($2)
	jal	0x00104f20
	nop	
	jal	0x00105278
	nop	
	move	$5,$16
	jal	0x00104140
	move	$4,$2
	jal	0x00105038
	lh	$4,2($17)
	jal	0x00104f48
	lh	$4,0($17)
	jal	0x00105278
	lw	$16,0x10($17)
	lw	$4,0xc($16)
	jal	0x00105f20
	move	$5,$2
	jal	0x0010ec98
	lw	$4,0x10($17)
	ld	$31,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	j	0x105268
	addiu	$29,$29,0x30
	nop	
	addiu	$29,$29,-0x10
	sd	$31,0($29)
	jal	0x0013eb50
	addiu	$4,$0,0x16
	move	$5,$2
	beqz	$5,loc_001bc8b8
	addiu	$2,$0,1
	lw	$2,0x15c($5)
loc_001bc890:
	lw	$3,0x800($2)
	lw	$4,0x14($3)
	beqz	$4,loc_001bc8b8
	move	$2,$0
	jal	0x0013ebe0
	move	$4,$5
	move	$5,$2
	bnel	$5,$0,loc_001bc890
	lw	$2,0x15c($5)
	addiu	$2,$0,1
loc_001bc8b8:
	beqz	$2,loc_001bc990
	move	$2,$0
	jal	0x0013eb50
	addiu	$4,$0,0x17
	move	$5,$2
	beqz	$5,loc_001bc900
	addiu	$2,$0,1
	lw	$2,0x15c($5)
loc_001bc8d8:
	lw	$3,0x800($2)
	lw	$4,0x14($3)
	beqz	$4,loc_001bc900
	move	$2,$0
	jal	0x0013ebe0
	move	$4,$5
	move	$5,$2
	bnel	$5,$0,loc_001bc8d8
	lw	$2,0x15c($5)
	addiu	$2,$0,1
loc_001bc900:
	beqz	$2,loc_001bc990
	move	$2,$0
	jal	0x0013eb50
	addiu	$4,$0,0x18
	move	$5,$2
	beqz	$5,loc_001bc948
	addiu	$2,$0,1
	lw	$2,0x15c($5)
loc_001bc920:
	lw	$3,0x800($2)
	lw	$4,0x14($3)
	beqz	$4,loc_001bc948
	move	$2,$0
	jal	0x0013ebe0
	move	$4,$5
	move	$5,$2
	bnel	$5,$0,loc_001bc920
	lw	$2,0x15c($5)
	addiu	$2,$0,1
loc_001bc948:
	beqz	$2,loc_001bc990
	move	$2,$0
	jal	0x0013eb50
	addiu	$4,$0,0x19
	move	$5,$2
	beqz	$5,loc_001bc990
	addiu	$2,$0,1
	lw	$2,0x15c($5)
loc_001bc968:
	lw	$3,0x800($2)
	lw	$4,0x14($3)
	beqz	$4,loc_001bc990
	move	$2,$0
	jal	0x0013ebe0
	move	$4,$5
	move	$5,$2
	bnel	$5,$0,loc_001bc968
	lw	$2,0x15c($5)
	addiu	$2,$0,1
loc_001bc990:
	ld	$31,0($29)
	jr	$31
	addiu	$29,$29,0x10
	.set	macro
	.set	reorder
	.end	type24_hA
