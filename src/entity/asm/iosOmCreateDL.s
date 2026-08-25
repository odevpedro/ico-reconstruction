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
	beqz	$18,loc_0013fcf0
	sd	$16,0($29)
	lui	$2,0x28
	addiu	$19,$2,0x1ab0
	lw	$3,-0x6724($28)
	andi	$2,$3,1
loc_0013fc30:
	beqz	$2,loc_0013fc58
	move	$4,$0
	lw	$2,0x48($18)
	beql	$2,$0,loc_0013fc5c
	srav	$2,$3,$4
	jalr	$2
	move	$4,$18
	lw	$3,-0x6724($28)
	move	$4,$0
	nop	
loc_0013fc58:
	srav	$2,$3,$4
loc_0013fc5c:
	andi	$2,$2,1
	beqz	$2,loc_0013fcd4
	addiu	$17,$4,1
	lw	$2,0x4c($18)
	srav	$2,$2,$4
	andi	$2,$2,1
	beqz	$2,loc_0013fcd4
	sll	$2,$4,2
	addu	$2,$2,$19
	lw	$16,0($2)
	beqz	$16,loc_0013fcd8
	move	$4,$17
	lw	$2,0x16c($16)
loc_0013fc90:
	beql	$2,$0,loc_0013fcc4
	lw	$16,0x34($16)
	lw	$3,0x50($18)
	lw	$2,0x50($16)
	and	$3,$3,$2
	beql	$3,$0,loc_0013fcc4
	lw	$16,0x34($16)
	lw	$2,0x48($16)
	beql	$2,$0,loc_0013fcc4
	lw	$16,0x34($16)
	jalr	$2
	move	$4,$16
	lw	$16,0x34($16)
loc_0013fcc4:
	bnel	$16,$0,loc_0013fc90
	lw	$2,0x16c($16)
	b	loc_0013fcd8
	move	$4,$17
loc_0013fcd4:
	move	$4,$17
loc_0013fcd8:
	slti	$2,$4,0x20
	bnez	$2,loc_0013fc58
	lw	$3,-0x6724($28)
	lw	$18,0x34($18)
	bnez	$18,loc_0013fc30
	andi	$2,$3,1
loc_0013fcf0:
	ld	$31,0x40($29)
	ld	$19,0x30($29)
	ld	$18,0x20($29)
	ld	$17,0x10($29)
	ld	$16,0($29)
	jr	$31
	addiu	$29,$29,0x50
	nop	
	.set	macro
	.set	reorder
	.end	iosOmCreateDL
