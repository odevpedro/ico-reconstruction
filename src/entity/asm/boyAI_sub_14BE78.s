	.text
	.p2align 3
	.globl	boyAI_sub_14BE78
	.ent	boyAI_sub_14BE78
boyAI_sub_14BE78:
	.frame	$sp,0,$31
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
	andi	$4,$8,0x20
	sd	$2,0x478($16)
	bnez	$4,loc_0014beb0
	sd	$3,0x480($16)
	lw	$2,0x164($18)
	lw	$3,0x678($2)
	jal	0x00263fb0
	lwc1	$f12,0x33c($3)
	lui	$1,0x56
	ld	$5,-0x7e18($1)
	jal	0x00263110
	move	$4,$2
	bgezl	$2,loc_0014bec8
	ld	$2,0x480($16)
loc_0014beb0:
	ld	$2,0x480($16)
	ori	$3,$0,0x8000
	dsll32	$3,$3,0xb
	or	$2,$2,$3
	sd	$2,0x480($16)
	ld	$2,0x480($16)
loc_0014bec8:
	ori	$3,$0,0x8000
	dsll32	$3,$3,8
	ori	$4,$0,0x8000
	dsll32	$4,$4,9
	ori	$7,$0,0x8000
	dsll32	$7,$7,7
	or	$2,$2,$3
	ori	$6,$0,0x8000
	.set	macro
	.set	reorder
	.end	boyAI_sub_14BE78
