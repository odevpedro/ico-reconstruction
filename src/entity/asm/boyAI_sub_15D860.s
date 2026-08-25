	.text
	.p2align 3
	.globl	boyAI_sub_15D860
	.ent	boyAI_sub_15D860
boyAI_sub_15D860:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	addiu	$29,$29,-0x40
	sw	$4,0($29)
	sd	$17,0x20($29)
	lw	$2,0($29)
	move	$17,$0
	sd	$31,0x30($29)
	sd	$16,0x10($29)
	lw	$16,0x164($2)
	lw	$4,0($29)
	jal	0x00106ab8
	addiu	$5,$16,0x4c0
	nop	
loc_0015d890:
	lw	$4,0($29)
	jal	0x0010a310
	nop	
	beqz	$2,loc_0015d8b8
	nop	
	bnez	$17,loc_0015d8b8
	nop	
	lw	$4,0x604($16)
	jal	0x001f1868
	addiu	$17,$0,1
loc_0015d8b8:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015d890
	nop	
	nop	
	addiu	$29,$29,-0x40
	sw	$4,0($29)
	sd	$17,0x20($29)
	lw	$2,0($29)
	move	$17,$0
	sd	$31,0x30($29)
	sd	$16,0x10($29)
	lw	$16,0x164($2)
	lw	$4,0($29)
	jal	0x00106ab8
	addiu	$5,$16,0x4d0
	nop	
loc_0015d908:
	lw	$4,0($29)
	jal	0x0010a310
	nop	
	beqz	$2,loc_0015d930
	nop	
	bnez	$17,loc_0015d930
	nop	
	lw	$4,0x608($16)
	jal	0x001f1868
	addiu	$17,$0,1
loc_0015d930:
	lw	$4,0($29)
	jal	0x0015bcc8
	addiu	$5,$0,0xb7
	jal	0x00203aa0
	addiu	$4,$0,1
	b	loc_0015d908
	nop	
	nop	
	addiu	$29,$29,-0x20
	sw	$4,0($29)
	sd	$31,0x10($29)
	jal	0x00203aa0
	.set	macro
	.set	reorder
	.end	boyAI_sub_15D860
