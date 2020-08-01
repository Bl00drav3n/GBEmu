void cpu_reset() {
	device.cpu.freq = 4096 * 1024; // NOTE: 4MHz
	device.cpu.registers.af = 0x01B0;
	device.cpu.registers.bc = 0x0013;
	device.cpu.registers.de = 0x00D8;
	device.cpu.registers.hl = 0x014D;
	device.cpu.registers.pc = 0x0100;
	device.cpu.registers.sp = 0xFFFE;
	device.cpu.ime = 0;
}

static FILE* debug_out;
void dump_cpu_state() {
	auto reg = device.cpu.registers;
	fprintf(debug_out, "pc:%04Xh sp:%04Xh af:%04Xh bc:%04Xh de:%04Xh hl:%04Xh\n",
		(uint32_t)(reg.pc),
		(uint32_t)(reg.sp),
		(uint32_t)(reg.af),
		(uint32_t)(reg.bc),
		(uint32_t)(reg.de),
		(uint32_t)(reg.hl)
	);
	fflush(debug_out);
}

void cpu_set_flag(cpu_flags_t flag, bool value) {
	if (value) {
		device.cpu.registers.f |= flag;
	}
	else {
		device.cpu.registers.f &= ~flag;
	}
}

u8 cpu_get_flag(cpu_flags_t flag) {
	return (device.cpu.registers.f & flag) ? 0x01 : 0x00;
}

u8 cpu_fetch() {
	return bus_read(device.cpu.registers.pc++);
}

void fetch_from_a() {
	device.cpu.from8 = &device.cpu.registers.a;
}

void fetch_from_b() {
	device.cpu.from8 = &device.cpu.registers.b;
}

void fetch_from_c() {
	device.cpu.from8 = &device.cpu.registers.c;
}

void fetch_from_d() {
	device.cpu.from8 = &device.cpu.registers.d;
}

void fetch_from_e() {
	device.cpu.from8 = &device.cpu.registers.e;
}

void fetch_from_h() {
	device.cpu.from8 = &device.cpu.registers.h;
}

void fetch_from_l() {
	device.cpu.from8 = &device.cpu.registers.l;
}

void fetch_from_bc() {
	device.cpu.from16 = &device.cpu.registers.bc;
}

void fetch_from_de() {
	device.cpu.from16 = &device.cpu.registers.de;
}

void fetch_from_hl() {
	device.cpu.from16 = &device.cpu.registers.hl;
}

void fetch_from_sp() {
	device.cpu.from16 = &device.cpu.registers.sp;
}

void store_to_a() {
	device.cpu.to8 = &device.cpu.registers.a;
}

void store_to_b() {
	device.cpu.to8 = &device.cpu.registers.b;
}

void store_to_c() {
	device.cpu.to8 = &device.cpu.registers.c;
}

void store_to_d() {
	device.cpu.to8 = &device.cpu.registers.d;
}

void store_to_e() {
	device.cpu.to8 = &device.cpu.registers.e;
}

void store_to_h() {
	device.cpu.to8 = &device.cpu.registers.h;
}

void store_to_l() {
	device.cpu.to8 = &device.cpu.registers.l;
}

void store_to_bc() {
	device.cpu.to16 = &device.cpu.registers.bc;
}

void store_to_de() {
	device.cpu.to16 = &device.cpu.registers.de;
}

void store_to_hl() {
	device.cpu.to16 = &device.cpu.registers.hl;
}

void store_to_sp() {
	device.cpu.to16 = &device.cpu.registers.sp;
}

void fetch_imm8() {
	device.cpu.imm8 = cpu_fetch();
	device.cpu.from8 = &device.cpu.imm8;
}

void fetch_imm16() {
	u16 lo = cpu_fetch();
	u16 hi = cpu_fetch();
	device.cpu.imm16 = (hi << 8) | lo;
	device.cpu.from16 = &device.cpu.imm16;
}

void fetch_hl() {
	u16 lo = device.cpu.registers.l;
	u16 hi = device.cpu.registers.h;
	u16 addr = (hi << 8) | lo;
	device.cpu.addr = addr;
	device.cpu.imm8 = bus_read(addr);
	device.cpu.reg8 = 0;
}

void IMP() {
}

void IMM8() {
	fetch_imm8();
}

void IMM16() {
	fetch_imm16();
}

void ADDR_IMM() {
	fetch_imm16();
	device.cpu.addr = device.cpu.imm16;
}

void IND() {
	u8 fetched = bus_read(device.cpu.registers.hl);
	device.cpu.imm8 = fetched;
}

void ADDR_HL() {
	device.cpu.addr = device.cpu.registers.hl;
}

void ADDR_HI_A() {
	fetch_imm8();
	device.cpu.addr = 0xFF00 + device.cpu.imm8;
	fetch_from_a();
}

void ADDR_A_HI() {
	fetch_imm8();
	device.cpu.addr = 0xFF00 + device.cpu.imm8;
	store_to_a();
}

void IND_BC_A() {
	device.cpu.addr = device.cpu.registers.bc;
	fetch_from_a();
}

void IND_DE_A() {
	device.cpu.addr = device.cpu.registers.de;
	fetch_from_a();
}

void IND_HL_IMM() {
	device.cpu.addr = device.cpu.registers.hl;
	fetch_imm8();
}

void IND_HLI_A() {
	device.cpu.addr = device.cpu.registers.hl++;
	fetch_from_a();
}

void IND_HLD_A() {
	device.cpu.addr = device.cpu.registers.hl--;
	fetch_from_a();
}

void IND_A_BC() {
	device.cpu.addr = device.cpu.registers.bc;
	store_to_a();
}

void IND_A_DE() {
	device.cpu.addr = device.cpu.registers.de;
	store_to_a();
}

void IND_A_HLI() {
	device.cpu.addr = device.cpu.registers.hl++;
	store_to_a();
}

void IND_A_HLD() {
	device.cpu.addr = device.cpu.registers.hl--;
	store_to_a();
}

void IND_HL() {
	fetch_hl();
}

void IND_A_HL() {
	fetch_hl();
	store_to_a();
}

void IND_B_HL() {
	fetch_hl();
	store_to_b();
}

void IND_C_HL() {
	fetch_hl();
	store_to_c();
}

void IND_D_HL() {
	fetch_hl();
	store_to_d();
}

void IND_E_HL() {
	fetch_hl();
	store_to_e();
}

void IND_H_HL() {
	fetch_hl();
	store_to_h();
}

void IND_L_HL() {
	fetch_hl();
	store_to_l();
}

void A_IMM() {
	fetch_imm8();
	store_to_a();
}

void B_IMM() {
	fetch_imm8();
	store_to_b();
}

void C_IMM() {
	fetch_imm8();
	store_to_c();
}

void D_IMM() {
	fetch_imm8();
	store_to_d();
}

void E_IMM() {
	fetch_imm8();
	store_to_e();
}

void H_IMM() {
	fetch_imm8();
	store_to_h();
}

void L_IMM() {
	fetch_imm8();
	store_to_l();
}

void BC_IMM() {
	fetch_imm16();
	store_to_bc();
}

void DE_IMM() {
	fetch_imm16();
	store_to_de();
}

void HL_IMM() {
	fetch_imm16();
	store_to_hl();
}

void SP_IMM() {
	fetch_imm16();
	store_to_sp();
}

void HL_BC() {
	fetch_from_bc();
	store_to_hl();
}

void HL_DE() {
	fetch_from_de();
	store_to_hl();
}

void HL_HL() {
	fetch_from_hl();
	store_to_hl();
}

void HL_SP() {
	fetch_from_sp();
	store_to_hl();
}

void SP_HL() {
	fetch_from_hl();
	store_to_sp();
}

void A() {
	device.cpu.reg8 = &device.cpu.registers.a;
	device.cpu.imm8 = *device.cpu.reg8;
}

void B() {
	device.cpu.reg8 = &device.cpu.registers.b;
	device.cpu.imm8 = *device.cpu.reg8;
}

void C() {
	device.cpu.reg8 = &device.cpu.registers.c;
	device.cpu.imm8 = *device.cpu.reg8;
}

void D() {
	device.cpu.reg8 = &device.cpu.registers.d;
	device.cpu.imm8 = *device.cpu.reg8;
}

void E() {
	device.cpu.reg8 = &device.cpu.registers.e;
	device.cpu.imm8 = *device.cpu.reg8;
}

void H() {
	device.cpu.reg8 = &device.cpu.registers.h;
	device.cpu.imm8 = *device.cpu.reg8;
}

void L() {
	device.cpu.reg8 = &device.cpu.registers.l;
	device.cpu.imm8 = *device.cpu.reg8;
}

void AF() {
	device.cpu.reg16 = &device.cpu.registers.af;
	device.cpu.imm16 = *device.cpu.reg16;
}

void BC() {
	device.cpu.reg16 = &device.cpu.registers.bc;
	device.cpu.imm16 = *device.cpu.reg16;
}

void DE() {
	device.cpu.reg16 = &device.cpu.registers.de;
	device.cpu.imm16 = *device.cpu.reg16;
}

void HL() {
	device.cpu.reg16 = &device.cpu.registers.hl;
	device.cpu.imm16 = *device.cpu.reg16;
}

void SP() {
	device.cpu.reg16 = &device.cpu.registers.sp;
	device.cpu.imm16 = *device.cpu.reg16;
}

void A_A() {
	fetch_from_a();
	store_to_a();
}

void A_B() {
	fetch_from_b();
	store_to_a();
}

void A_C() {
	fetch_from_c();
	store_to_a();
}

void A_D() {
	fetch_from_d();
	store_to_a();
}

void A_E() {
	fetch_from_e();
	store_to_a();
}

void A_H() {
	fetch_from_h();
	store_to_a();
}

void A_L() {
	fetch_from_l();
	store_to_a();
}

void B_A() {
	fetch_from_a();
	store_to_b();
}

void B_B() {
	fetch_from_b();
	store_to_b();
}

void B_C() {
	fetch_from_c();
	store_to_b();
}

void B_D() {
	fetch_from_d();
	store_to_b();
}

void B_E() {
	fetch_from_e();
	store_to_b();
}

void B_H() {
	fetch_from_h();
	store_to_b();
}

void B_L() {
	fetch_from_l();
	store_to_b();
}

void C_A() {
	fetch_from_a();
	store_to_c();
}

void C_B() {
	fetch_from_b();
	store_to_c();
}

void C_C() {
	fetch_from_c();
	store_to_c();
}

void C_D() {
	fetch_from_d();
	store_to_c();
}

void C_E() {
	fetch_from_e();
	store_to_c();
}

void C_H() {
	fetch_from_h();
	store_to_c();
}

void C_L() {
	fetch_from_l();
	store_to_c();
}

void D_A() {
	fetch_from_a();
	store_to_d();
}

void D_B() {
	fetch_from_b();
	store_to_d();
}

void D_C() {
	fetch_from_c();
	store_to_d();
}

void D_D() {
	fetch_from_d();
	store_to_d();
}

void D_E() {
	fetch_from_e();
	store_to_d();
}

void D_H() {
	fetch_from_h();
	store_to_d();
}

void D_L() {
	fetch_from_l();
	store_to_d();
}

void E_A() {
	fetch_from_a();
	store_to_e();
}

void E_B() {
	fetch_from_b();
	store_to_e();
}

void E_C() {
	fetch_from_c();
	store_to_e();
}

void E_D() {
	fetch_from_d();
	store_to_e();
}

void E_E() {
	fetch_from_e();
	store_to_e();
}

void E_H() {
	fetch_from_h();
	store_to_e();
}

void E_L() {
	fetch_from_l();
	store_to_e();
}

void H_A() {
	fetch_from_a();
	store_to_h();
}

void H_B() {
	fetch_from_b();
	store_to_h();
}

void H_C() {
	fetch_from_c();
	store_to_h();
}

void H_D() {
	fetch_from_d();
	store_to_h();
}

void H_E() {
	fetch_from_e();
	store_to_h();
}

void H_H() {
	fetch_from_h();
	store_to_h();
}

void H_L() {
	fetch_from_l();
	store_to_h();
}

void L_A() {
	fetch_from_a();
	store_to_l();
}

void L_B() {
	fetch_from_b();
	store_to_l();
}

void L_C() {
	fetch_from_c();
	store_to_l();
}

void L_D() {
	fetch_from_d();
	store_to_l();
}

void L_E() {
	fetch_from_e();
	store_to_l();
}

void L_H() {
	fetch_from_h();
	store_to_l();
}

void L_L() {
	fetch_from_l();
	store_to_l();
}

void ADDR_SP() {
	fetch_imm16();
	device.cpu.addr = device.cpu.imm16;
	fetch_from_sp();
}

void IND_HL_A() {
	device.cpu.addr = device.cpu.registers.hl;
	fetch_from_a();
}

void IND_HL_B() {
	device.cpu.addr = device.cpu.registers.hl;
	fetch_from_b();
}

void IND_HL_C() {
	device.cpu.addr = device.cpu.registers.hl;
	fetch_from_c();
}

void IND_HL_D() {
	device.cpu.addr = device.cpu.registers.hl;
	fetch_from_d();
}

void IND_HL_E() {
	device.cpu.addr = device.cpu.registers.hl;
	fetch_from_e();
}

void IND_HL_H() {
	device.cpu.addr = device.cpu.registers.hl;
	fetch_from_h();
}

void IND_HL_L() {
	device.cpu.addr = device.cpu.registers.hl;
	fetch_from_l();
}

void ADDR_A() {
	fetch_imm16();
	device.cpu.addr = device.cpu.imm16;
	device.cpu.to8 = &device.cpu.registers.a;
	device.cpu.from8 = &device.cpu.registers.a;
}

void IND_C_A() {
	device.cpu.addr = 0xff00 | device.cpu.registers.c;
	device.cpu.to8 = &device.cpu.registers.a;
	device.cpu.from8 = &device.cpu.registers.a;
}

u8 NOP() {
	return 0;
}

u8 STOP() {
	device.cpu.stopped = true;
	return 0;
}

u8 MOVS() {
	*device.cpu.to8 = *device.cpu.from8;
	return 0;
}

u8 STOS() {
	bus_write(device.cpu.addr, *device.cpu.from8);
	return 0;
}

u8 LOAS() {
	*device.cpu.to8 = bus_read(device.cpu.addr);
	return 0;
}

u8 STOD() {
	u8 lo = *device.cpu.from16 >> 8;
	u8 hi = *device.cpu.from16 & 0xff;
	bus_write(device.cpu.addr, lo);
	bus_write(device.cpu.addr + 1, hi);
	return 0;
}

u8 LOAD() {

	u8 lo = bus_read(device.cpu.addr);
	u8 hi = bus_read(device.cpu.addr + 1);
	*device.cpu.to16 = (hi << 8) | lo;
	return 0;
}

u8 MOVD() {
	*device.cpu.to16 = *device.cpu.from16;
	return 0;
}

bool check_half_carry_add(u8 a, u8 b, u8 cy = 0) {
	return (u16)((a & 0xF) + (b & 0xF) + cy) > 0xF;
}

bool check_half_carry_sub(u8 a, u8 b, u8 cy = 0) {
	return (u16)((a & 0xF) - (b & 0xF) - cy) > 0xF;
}

bool check_half_carry16(u16 a, u16 b) {
	return (u16)((a & 0xFFF) + (b & 0xFFF)) > 0xFFF;
}

u8 INCS() {
	u8 value = *device.cpu.reg8;
	cpu_set_flag(FLAG_ZF, ((value + 1) & 0xFF) == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, check_half_carry_add(value, 1));
	*device.cpu.reg8 = value + 1;
	return 0;
}

u8 INCD() {
	(*device.cpu.reg16)++;
	return 0;
}

u8 INC_IND_HL() {
	u8 fetched = bus_read(device.cpu.registers.hl);
	cpu_set_flag(FLAG_ZF, ((fetched + 1) & 0xFF) == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, check_half_carry_add(fetched, 1));
	bus_write(device.cpu.registers.hl, fetched + 1);
	return 0;
}

u8 DECS() {
	u8 value = *device.cpu.reg8;
	cpu_set_flag(FLAG_ZF, ((value - 1) & 0xff) == 0);
	cpu_set_flag(FLAG_N, 1);
	cpu_set_flag(FLAG_H, check_half_carry_sub(value, 1));
	*device.cpu.reg8 = value - 1;
	return 0;
}

u8 DECD() {
	(*device.cpu.reg16)--;
	return 0;
}

u8 DEC_IND_HL() {
	u8 fetched = bus_read(device.cpu.registers.hl);
	cpu_set_flag(FLAG_ZF, ((fetched - 1) & 0xFF) == 0);
	cpu_set_flag(FLAG_N, 1);
	cpu_set_flag(FLAG_H, check_half_carry_sub(fetched, 1));
	bus_write(device.cpu.registers.hl, fetched - 1);
	return 0;
}

u8 JP() {
	device.cpu.registers.pc = device.cpu.addr;
	return 0;
}

u8 JPNZ() {
	if (!cpu_get_flag(FLAG_ZF)) {
		JP();
		return 4;
	}
	return 0;
}

u8 JPZ() {
	if (cpu_get_flag(FLAG_ZF)) {
		JP();
		return 4;
	}
	return 0;
}

u8 JPNC() {
	if (!cpu_get_flag(FLAG_CY)) {
		JP();
		return 4;
	}
	return 0;
}

u8 JPC() {
	if (cpu_get_flag(FLAG_CY)) {
		JP();
		return 4;
	}
	return 0;
}

u8 DI() {
	device.cpu.disable_interrupt = true;
	return 0;
}

u8 EI() {
	device.cpu.enable_interrupt = true;
	return 0;
}

u8 HALT() {
	device.cpu.halted = true;
	return 0;
}

u8 POP() {
	u16 lo = bus_read(device.cpu.registers.sp++);
	u16 hi = bus_read(device.cpu.registers.sp++);
	u16 data = (hi << 8) | lo;
	*device.cpu.reg16 = (device.cpu.reg16 == &device.cpu.registers.af) ? data & 0xFFF0 : data;
	return 0;
}

u8 PUSH() {
	u16 data = *device.cpu.reg16;
	u8 hi = data >> 8;
	u8 lo = data & 0xFF;
	bus_write(--device.cpu.registers.sp, hi);
	bus_write(--device.cpu.registers.sp, lo);
	return 0;
}

void PUSH_PC() {
	device.cpu.reg16 = &device.cpu.registers.pc;
	PUSH();
}

u8 CALL() {
	PUSH_PC();
	device.cpu.registers.pc = device.cpu.imm16;
	return 0;
}

u8 CALL_NZ() {
	if (!cpu_get_flag(FLAG_ZF)) {
		CALL();
		return 12;
	}
	return 0;
}

u8 CALL_Z() {
	if (cpu_get_flag(FLAG_ZF)) {
		CALL();
		return 12;
	}
	return 0;
}

u8 CALL_NC() {
	if (!cpu_get_flag(FLAG_CY)) {
		CALL();
		return 12;
	}
	return 0;
}

u8 CALL_C() {
	if (cpu_get_flag(FLAG_CY)) {
		CALL();
		return 12;
	}
	return 0;
}

u8 ADD() {
	u16 tmp = device.cpu.registers.a + device.cpu.imm8;
	cpu_set_flag(FLAG_ZF, (u8)(tmp & 0xFF) == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, check_half_carry_add(device.cpu.registers.a, device.cpu.imm8));
	cpu_set_flag(FLAG_CY, tmp & 0x0100);
	device.cpu.registers.a = (u8)(tmp & 0xFF);
	return 0;
}

u8 ADDSP() {
	int32_t offset = (int8_t)device.cpu.imm8;
	cpu_set_flag(FLAG_CY, (device.cpu.registers.sp & 0xFF) + (u8)(offset & 0xFF) > 0xFF);
	cpu_set_flag(FLAG_CY, (device.cpu.registers.sp & 0x0F) + (offset & 0x0F) > 0x0F);
	device.cpu.registers.sp = (u16)(device.cpu.registers.sp + offset);
#if 0
	uint32_t tmp;
	if (device.cpu.imm8 & 0x80) {
		tmp = device.cpu.registers.sp + (0xffffff00 | device.cpu.imm8);
		cpu_set_flag(FLAG_H, check_half_carry_sub(device.cpu.registers.sp & 0xFF, device.cpu.imm8));
		cpu_set_flag(FLAG_CY, device.cpu.registers.sp - (0xff00 | device.cpu.imm8) < 0);
	}
	else {
		tmp = device.cpu.registers.sp + device.cpu.imm8;
		cpu_set_flag(FLAG_H, check_half_carry_add(device.cpu.registers.sp & 0xFF, device.cpu.imm8));
		cpu_set_flag(FLAG_CY, tmp & 0x10000);
	}
	device.cpu.registers.sp = tmp & 0xFFFF;
#endif
	cpu_set_flag(FLAG_ZF, 0);
	cpu_set_flag(FLAG_N, 0);
	return 0;
}

u8 LDHLSP() {
	int32_t offset = (int8_t)device.cpu.imm8;
	cpu_set_flag(FLAG_CY, (device.cpu.registers.sp & 0xFF) + (u8)(offset & 0xFF) > 0xFF);
	cpu_set_flag(FLAG_CY, (device.cpu.registers.sp & 0x0F) + (offset & 0x0F) > 0x0F);
	u16 res = device.cpu.registers.sp + offset;
	device.cpu.registers.l = res & 0xFF;
	device.cpu.registers.h = res >> 8;
#if 0
	uint32_t tmp;
	if (device.cpu.imm8 & 0x80) {
		tmp = device.cpu.registers.sp + (0xffffff00 | device.cpu.imm8);
		cpu_set_flag(FLAG_H, check_half_carry_sub(device.cpu.registers.sp & 0xFF, device.cpu.imm8));
		cpu_set_flag(FLAG_CY, device.cpu.registers.sp - (0xff00 | device.cpu.imm8) < 0);
	}
	else {
		tmp = device.cpu.registers.sp + device.cpu.imm8;
		cpu_set_flag(FLAG_H, check_half_carry_add(device.cpu.registers.sp & 0xFF, device.cpu.imm8));
		cpu_set_flag(FLAG_CY, tmp & 0x10000);
	}
	device.cpu.registers.hl = tmp & 0xFFFF;
#endif
	cpu_set_flag(FLAG_ZF, 0);
	cpu_set_flag(FLAG_N, 0);
	return 0;
}

u8 ADD16() {
	uint32_t tmp = *device.cpu.from16 + *device.cpu.to16;
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, check_half_carry16(*device.cpu.from16, *device.cpu.to16));
	cpu_set_flag(FLAG_CY, tmp & 0x10000);
	*device.cpu.to16 = (u16)(tmp & 0xFFFF);
	return 0;
}

u8 SUB() {
	u16 tmp = device.cpu.registers.a - device.cpu.imm8;
	cpu_set_flag(FLAG_ZF, (u8)(tmp & 0xFF) == 0);
	cpu_set_flag(FLAG_N, 1);
	cpu_set_flag(FLAG_H, check_half_carry_sub(device.cpu.registers.a, device.cpu.imm8));
	cpu_set_flag(FLAG_CY, tmp > 0xFF);
	device.cpu.registers.a = (u8)(tmp & 0xFF);
	return 0;
}

u8 ADC() {
	u8 cy = cpu_get_flag(FLAG_CY) ? 1 : 0;
	u16 tmp1 = device.cpu.registers.a + device.cpu.imm8;
	u16 tmp2 = tmp1 + cy;
	cpu_set_flag(FLAG_ZF, (u8)(tmp2 & 0xFF) == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, check_half_carry_add(device.cpu.registers.a, device.cpu.imm8, cy));
	cpu_set_flag(FLAG_CY, tmp2 & 0x0100);
	device.cpu.registers.a = (u8)(tmp2 & 0xFF);
	return 0;
}

u8 SBC() {
	u8 cy = cpu_get_flag(FLAG_CY) ? 1 : 0;
	u16 tmp = device.cpu.registers.a - device.cpu.imm8 - cy;
	cpu_set_flag(FLAG_ZF, (u8)(tmp & 0xFF) == 0);
	cpu_set_flag(FLAG_N, 1);
	cpu_set_flag(FLAG_CY, tmp > 0xFF);
	cpu_set_flag(FLAG_H, check_half_carry_sub(device.cpu.registers.a, device.cpu.imm8, cy));
	device.cpu.registers.a = (u8)(tmp & 0xFF);
	return 0;
}

u8 AND() {
	u8 value = device.cpu.registers.a & device.cpu.imm8;
	device.cpu.registers.a = value;
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 1);
	cpu_set_flag(FLAG_CY, 0);
	return 0;
}

u8 OR() {
	u8 value = device.cpu.registers.a | device.cpu.imm8;
	device.cpu.registers.a = value;
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, 0);
	return 0;
}

u8 XOR() {
	u8 value = device.cpu.registers.a ^ device.cpu.imm8;
	device.cpu.registers.a = value;
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, 0);
	return 0;
}

u8 CP() {
	u16 tmp = device.cpu.registers.a + ~device.cpu.imm8 + 1;
	u8 value = (u8)(tmp & 0xFF);
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 1);
	cpu_set_flag(FLAG_H, check_half_carry_sub(device.cpu.registers.a, device.cpu.imm8));
	cpu_set_flag(FLAG_CY, tmp & 0x0100);
	return 0;
}

u8 RST00() {
	PUSH_PC();
	device.cpu.registers.pc = 0x0000;
	return 0;
}

u8 RST08() {
	PUSH_PC();
	device.cpu.registers.pc = 0x0008;
	return 0;
}

u8 RST10() {
	PUSH_PC();
	device.cpu.registers.pc = 0x0010;
	return 0;
}

u8 RST18() {
	PUSH_PC();
	device.cpu.registers.pc = 0x0018;
	return 0;
}

u8 RST20() {
	PUSH_PC();
	device.cpu.registers.pc = 0x0020;
	return 0;
}

u8 RST28() {
	PUSH_PC();
	device.cpu.registers.pc = 0x0028;
	return 0;
}

u8 RST30() {
	PUSH_PC();
	device.cpu.registers.pc = 0x0030;
	return 0;
}

u8 RST38() {
	PUSH_PC();
	device.cpu.registers.pc = 0x0038;
	return 0;
}

u8 RET() {
	device.cpu.reg16 = &device.cpu.registers.pc;
	POP();
	return 0;
}

u8 RETI() {
	RET();
	device.cpu.ime = 1;
	return 0;
}

u8 RETZ() {
	if (cpu_get_flag(FLAG_ZF)) {
		RET();
		return 12;
	}
	return 0;
}

u8 RETC() {
	if (cpu_get_flag(FLAG_CY)) {
		RET();
		return 12;
	}
	return 0;
}

u8 RETNZ() {
	if (!cpu_get_flag(FLAG_ZF)) {
		RET();
		return 12;
	}
	return 0;
}

u8 RETNC() {
	if (!cpu_get_flag(FLAG_CY)) {
		RET();
		return 12;
	}
	return 0;
}

u8 JR() {
	u16 offset = ((device.cpu.imm8 & 0x80) ? 0xff00 : 0x0000) | device.cpu.imm8;
	device.cpu.registers.pc += offset;
	return 0;
}

u8 JRNZ() {
	if (!cpu_get_flag(FLAG_ZF)) {
		JR();
		return 4;
	}
	return 0;
}

u8 JRZ() {
	if (cpu_get_flag(FLAG_ZF)) {
		JR();
		return 4;
	}
	return 0;
}

u8 JRNC() {
	if (!cpu_get_flag(FLAG_CY)) {
		JR();
		return 4;
	}
	return 0;
}

u8 JRC() {
	if (cpu_get_flag(FLAG_CY)) {
		JR();
		return 4;
	}
	return 0;
}

u8 RLCA() {
	u8 value = device.cpu.registers.a;
	u8 cy = value & 0x80;
	value = (value << 1) | (cy >> 7);
	cpu_set_flag(FLAG_ZF, 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
	device.cpu.registers.a = value;
	return 0;
}

u8 RLA() {
	u8 value = device.cpu.registers.a;
	u8 cy = value & 0x80;
	value = (value << 1) | cpu_get_flag(FLAG_CY);
	cpu_set_flag(FLAG_ZF, 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
	device.cpu.registers.a = value;
	return 0;
}

u8 RRCA() {
	u8 value = device.cpu.registers.a;
	u8 cy = value & 0x01;
	value = (value >> 1) | (cy << 7);
	cpu_set_flag(FLAG_ZF, 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
	device.cpu.registers.a = value;
	return 0;
}

u8 RRA() {
	u8 value = device.cpu.registers.a;
	u8 cy = value & 0x01;
	value = (value >> 1) | (cpu_get_flag(FLAG_CY) << 7);
	cpu_set_flag(FLAG_ZF, 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
	device.cpu.registers.a = value;
	return 0;
}

u8 CPL() {
	device.cpu.registers.a = ~device.cpu.registers.a;
	cpu_set_flag(FLAG_N, 1);
	cpu_set_flag(FLAG_H, 1);
	return 0;
}

u8 CCF() {
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, !cpu_get_flag(FLAG_CY));
	return 0;
}

u8 SCF() {
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, 1);
	return 0;
}

u8 DAA() {
#if 0
	int value = device.cpu.registers.a;
	if (!cpu_get_flag(FLAG_N)) {
		if (cpu_get_flag(FLAG_H) || (value & 0x0F) > 9) {
			value += 6;
		}
		if (cpu_get_flag(FLAG_CY) || value > 0x9F) {
			value += 0x60;
		}
	}
	else {
		if (cpu_get_flag(FLAG_H)) {
			value -= 6;
			if (!cpu_get_flag(FLAG_CY))
				value &= 0xFF;
		}
		if (cpu_get_flag(FLAG_CY))
			value -= 0x60;
	}
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_CY, value & 0x0100);
	device.cpu.registers.a = value;
#else
	// TODO: Seems to fail on several test ROMs
	u8 value = device.cpu.registers.a;
	u8 lo = value & 0x0f;
	u8 hi = (value & 0xf0) >> 4;
	u8 cy = 0;
	if (cpu_get_flag(FLAG_N)) {
		if (lo >= 10) {
			lo = 9;
		}
		if (hi >= 10) {
			hi = 9;
			cy = 1;
		}
	}
	else {
		if (lo >= 10) {
			lo -= 10;
			hi++;
		}
		if (hi >= 10) {
			hi -= 10;
			cy = 1;
		}
	}
	value = (hi << 4) | lo;
	device.cpu.registers.a = value;
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
#endif
	return 0;
}

void BIT0() {
	device.cpu.cb_bitmask = 0x01 << 0;
}

void BIT1() {
	device.cpu.cb_bitmask = 0x01 << 1;
}

void BIT2() {
	device.cpu.cb_bitmask = 0x01 << 2;
}

void BIT3() {
	device.cpu.cb_bitmask = 0x01 << 3;
}

void BIT4() {
	device.cpu.cb_bitmask = 0x01 << 4;
}

void BIT5() {
	device.cpu.cb_bitmask = 0x01 << 5;
}

void BIT6() {
	device.cpu.cb_bitmask = 0x01 << 6;
}

void BIT7() {
	device.cpu.cb_bitmask = 0x01 << 7;
}

void BIT() {
	cpu_set_flag(FLAG_ZF, (device.cpu.imm8 & device.cpu.cb_bitmask) == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 1);
}

void RES() {
	if (device.cpu.reg8) {
		*device.cpu.reg8 = *device.cpu.reg8 & ~device.cpu.cb_bitmask;
	}
	else {
		bus_write(device.cpu.addr, device.cpu.imm8 & ~device.cpu.cb_bitmask);
	}
}

void SET() {
	if (device.cpu.reg8) {
		*device.cpu.reg8 = *device.cpu.reg8 | device.cpu.cb_bitmask;
	}
	else {
		bus_write(device.cpu.addr, device.cpu.imm8 | device.cpu.cb_bitmask);
	}
}

void SWAP() {
	u8 value;
	if (device.cpu.reg8) {
		value = ((*device.cpu.reg8) >> 4) | ((*device.cpu.reg8) << 4);
		*device.cpu.reg8 = value;
	}
	else {
		value = (device.cpu.imm8 >> 4) | (device.cpu.imm8 << 4);
		bus_write(device.cpu.addr, value);
	}
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, 0);
}

void RLC() {
	u8 cy, value;
	if (device.cpu.reg8) {
		cy = *device.cpu.reg8 & 0x80;
		value = ((*device.cpu.reg8) << 1) | (cy >> 7);
		*device.cpu.reg8 = value;
	}
	else {
		cy = device.cpu.imm8 & 0x80;
		value = (device.cpu.imm8 << 1) | (cy >> 7);
		bus_write(device.cpu.addr, value);
	}
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
}

void RRC() {
	u8 cy, value;
	if (device.cpu.reg8) {
		cy = *device.cpu.reg8 & 0x01;
		value = ((*device.cpu.reg8) >> 1) | (cy << 7);
		*device.cpu.reg8 = value;
	}
	else {
		cy = device.cpu.imm8 & 0x01;
		value = (device.cpu.imm8 >> 1) | (cy << 7);
		bus_write(device.cpu.addr, value);
	}
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
}

void RL() {
	u8 cy, value;
	if (device.cpu.reg8) {
		cy = *device.cpu.reg8 & 0x80;
		value = ((*device.cpu.reg8) << 1) | (cpu_get_flag(FLAG_CY) ? 0x01 : 0x00);
		*device.cpu.reg8 = value;
	}
	else {
		cy = device.cpu.imm8 & 0x80;
		value = (device.cpu.imm8 << 1) | (cpu_get_flag(FLAG_CY) ? 0x01 : 0x00);
		bus_write(device.cpu.addr, value);
	}
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
}

void RR() {
	u8 cy, value;
	if (device.cpu.reg8) {
		cy = *device.cpu.reg8 & 0x01;
		value = ((*device.cpu.reg8) >> 1) | (cpu_get_flag(FLAG_CY) ? 0x80 : 0x00);
		*device.cpu.reg8 = value;
	}
	else {
		cy = device.cpu.imm8 & 0x01;
		value = (device.cpu.imm8 >> 1) | (cpu_get_flag(FLAG_CY) ? 0x80 : 0x00);
		bus_write(device.cpu.addr, value);
	}
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
}

void SLA() {
	u8 cy, value;
	if (device.cpu.reg8) {
		cy = *device.cpu.reg8 & 0x80;
		value = (*device.cpu.reg8) << 1;
		*device.cpu.reg8 = value;
	}
	else {
		cy = device.cpu.imm8 & 0x80;
		value = device.cpu.imm8 << 1;
		bus_write(device.cpu.addr, value);
	}
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
}

void SRA() {
	u8 cy, value;
	if (device.cpu.reg8) {
		cy = *device.cpu.reg8 & 0x01;
		value = ((*device.cpu.reg8) >> 1) | (*device.cpu.reg8 & 0x80);
		*device.cpu.reg8 = value;
	}
	else {
		cy = device.cpu.imm8 & 0x01;
		value = (device.cpu.imm8 >> 1) | (device.cpu.imm8 & 0x80);
		bus_write(device.cpu.addr, value);
	}
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
}

void SRL() {
	u8 cy, value;
	if (device.cpu.reg8) {
		cy = *device.cpu.reg8 & 0x01;
		value = (*device.cpu.reg8) >> 1;
		*device.cpu.reg8 = value;
	}
	else {
		cy = device.cpu.imm8 & 0x01;
		value = device.cpu.imm8 >> 1;
		bus_write(device.cpu.addr, value);
	}
	cpu_set_flag(FLAG_ZF, value == 0);
	cpu_set_flag(FLAG_N, 0);
	cpu_set_flag(FLAG_H, 0);
	cpu_set_flag(FLAG_CY, cy);
}

struct cb_instr_t {
	const char* name;
	void(*bit)(void);
	void(*reg)(void);
	void(*op)(void);
	u8 cycles;
};
static cb_instr_t cb_instructions[256] = {
	/*                        x0                        x1                        x2                        x3                       x4                        x5                                  x6                        x7                        x8                        x9                        xA                        xB                        XC                        xD                                 xE                        xF */
		   {"RLC B",IMP,B,RLC,8},    {"RLC C",IMP,C,RLC,8},    {"RLC D",IMP,D,RLC,8},    {"RLC E",IMP,E,RLC,8},   {"RLC H",IMP,H,RLC,8},    {"RLC L",IMP,L,RLC,8},     {"RLC (HL)",IMP,IND_HL,RLC,16},    {"RLC A",IMP,A,RLC,8},    {"RRC B",IMP,B,RRC,8},    {"RRC C",IMP,C,RRC,8},    {"RRC D",IMP,D,RRC,8},    {"RRC E",IMP,E,RRC,8},    {"RRC H",IMP,H,RRC,8},    {"RRC L",IMP,L,RRC,8},    {"RRC (HL)",IMP,IND_HL,RRC,16},    {"RRC A",IMP,A,RRC,8},
			 {"RL B",IMP,B,RL,8},      {"RL C",IMP,C,RL,8},      {"RL D",IMP,D,RL,8},      {"RL E",IMP,E,RL,8},     {"RL H",IMP,H,RL,8},      {"RL L",IMP,L,RL,8},       {"RL (HL)",IMP,IND_HL,RL,16},      {"RL A",IMP,A,RL,8},      {"RR B",IMP,B,RR,8},      {"RR C",IMP,C,RR,8},      {"RR D",IMP,D,RR,8},      {"RR E",IMP,E,RR,8},      {"RR H",IMP,H,RR,8},      {"RR L",IMP,L,RR,8},      {"RR (HL)",IMP,IND_HL,RR,16},      {"RR A",IMP,A,RR,8},
		   {"SLA B",IMP,B,SLA,8},    {"SLA C",IMP,C,SLA,8},    {"SLA D",IMP,D,SLA,8},    {"SLA E",IMP,E,SLA,8},   {"SLA H",IMP,H,SLA,8},    {"SLA L",IMP,L,SLA,8},     {"SLA (HL)",IMP,IND_HL,SLA,16},    {"SLA A",IMP,A,SLA,8},    {"SRA B",IMP,B,SRA,8},    {"SRA C",IMP,C,SRA,8},    {"SRA D",IMP,D,SRA,8},    {"SRA E",IMP,E,SRA,8},    {"SRA H",IMP,H,SRA,8},    {"SRA L",IMP,L,SRA,8},    {"SRA (HL)",IMP,IND_HL,SRA,16},    {"SRA A",IMP,A,SRA,8},
		 {"SWAP B",IMP,B,SWAP,8},  {"SWAP C",IMP,C,SWAP,8},  {"SWAP D",IMP,D,SWAP,8},  {"SWAP E",IMP,E,SWAP,8}, {"SWAP H",IMP,H,SWAP,8},  {"SWAP L",IMP,L,SWAP,8},   {"SWAP (HL)",IMP,IND_HL,SWAP,16},  {"SWAP A",IMP,A,SWAP,8},    {"SRL B",IMP,B,SRL,8},    {"SRL C",IMP,C,SRL,8},    {"SRL D",IMP,D,SRL,8},    {"SRL E",IMP,E,SRL,8},    {"SRL H",IMP,H,SRL,8},    {"SRL L",IMP,L,SRL,8},    {"SRL (HL)",IMP,IND_HL,SRL,16},    {"SRL A",IMP,A,SRL,8},
		{"BIT 0,B",BIT0,B,BIT,8}, {"BIT 0,C",BIT0,C,BIT,8}, {"BIT 0,D",BIT0,D,BIT,8}, {"BIT 0,E",BIT0,E,BIT,8}, {"BIT 0,H",BIT0,H,BIT,8}, {"BIT 0,L",BIT0,L,BIT,8}, {"BIT 0,(HL)",BIT0,IND_HL,BIT,16}, {"BIT 0,A",BIT0,A,BIT,8}, {"BIT 1,B",BIT1,B,BIT,8}, {"BIT 1,C",BIT1,C,BIT,8}, {"BIT 1,D",BIT1,D,BIT,8}, {"BIT 1,E",BIT1,E,BIT,8}, {"BIT 1,H",BIT1,H,BIT,8}, {"BIT 1,L",BIT1,L,BIT,8}, {"BIT 1,(HL)",BIT1,IND_HL,BIT,16}, {"BIT 1,A",BIT1,A,BIT,8},
		{"BIT 2,B",BIT2,B,BIT,8}, {"BIT 2,C",BIT2,C,BIT,8}, {"BIT 2,D",BIT2,D,BIT,8}, {"BIT 2,E",BIT2,E,BIT,8}, {"BIT 2,H",BIT2,H,BIT,8}, {"BIT 2,L",BIT2,L,BIT,8}, {"BIT 2,(HL)",BIT2,IND_HL,BIT,16}, {"BIT 2,A",BIT2,A,BIT,8}, {"BIT 3,B",BIT3,B,BIT,8}, {"BIT 3,C",BIT3,C,BIT,8}, {"BIT 3,D",BIT3,D,BIT,8}, {"BIT 3,E",BIT3,E,BIT,8}, {"BIT 3,H",BIT3,H,BIT,8}, {"BIT 3,L",BIT3,L,BIT,8}, {"BIT 3,(HL)",BIT3,IND_HL,BIT,16}, {"BIT 3,A",BIT3,A,BIT,8},
		{"BIT 4,B",BIT4,B,BIT,8}, {"BIT 4,C",BIT4,C,BIT,8}, {"BIT 4,D",BIT4,D,BIT,8}, {"BIT 4,E",BIT4,E,BIT,8}, {"BIT 4,H",BIT4,H,BIT,8}, {"BIT 4,L",BIT4,L,BIT,8}, {"BIT 4,(HL)",BIT4,IND_HL,BIT,16}, {"BIT 4,A",BIT4,A,BIT,8}, {"BIT 5,B",BIT5,B,BIT,8}, {"BIT 5,C",BIT5,C,BIT,8}, {"BIT 5,D",BIT5,D,BIT,8}, {"BIT 5,E",BIT5,E,BIT,8}, {"BIT 5,H",BIT5,H,BIT,8}, {"BIT 5,L",BIT5,L,BIT,8}, {"BIT 5,(HL)",BIT5,IND_HL,BIT,16}, {"BIT 5,A",BIT5,A,BIT,8},
		{"BIT 6,B",BIT6,B,BIT,8}, {"BIT 6,C",BIT6,C,BIT,8}, {"BIT 6,D",BIT6,D,BIT,8}, {"BIT 6,E",BIT6,E,BIT,8}, {"BIT 6,H",BIT6,H,BIT,8}, {"BIT 6,L",BIT6,L,BIT,8}, {"BIT 6,(HL)",BIT6,IND_HL,BIT,16}, {"BIT 6,A",BIT6,A,BIT,8}, {"BIT 7,B",BIT7,B,BIT,8}, {"BIT 7,C",BIT7,C,BIT,8}, {"BIT 7,D",BIT7,D,BIT,8}, {"BIT 7,E",BIT7,E,BIT,8}, {"BIT 7,H",BIT7,H,BIT,8}, {"BIT 7,L",BIT7,L,BIT,8}, {"BIT 7,(HL)",BIT7,IND_HL,BIT,16}, {"BIT 7,A",BIT7,A,BIT,8},
		{"RES 0,B",BIT0,B,RES,8}, {"RES 0,C",BIT0,C,RES,8}, {"RES 0,D",BIT0,D,RES,8}, {"RES 0,E",BIT0,E,RES,8}, {"RES 0,H",BIT0,H,RES,8}, {"RES 0,L",BIT0,L,RES,8}, {"RES 0,(HL)",BIT0,IND_HL,RES,16}, {"RES 0,A",BIT0,A,RES,8}, {"RES 1,B",BIT1,B,RES,8}, {"RES 1,C",BIT1,C,RES,8}, {"RES 1,D",BIT1,D,RES,8}, {"RES 1,E",BIT1,E,RES,8}, {"RES 1,H",BIT1,H,RES,8}, {"RES 1,L",BIT1,L,RES,8}, {"RES 1,(HL)",BIT1,IND_HL,RES,16}, {"RES 1,A",BIT1,A,RES,8},
		{"RES 2,B",BIT2,B,RES,8}, {"RES 2,C",BIT2,C,RES,8}, {"RES 2,D",BIT2,D,RES,8}, {"RES 2,E",BIT2,E,RES,8}, {"RES 2,H",BIT2,H,RES,8}, {"RES 2,L",BIT2,L,RES,8}, {"RES 2,(HL)",BIT2,IND_HL,RES,16}, {"RES 2,A",BIT2,A,RES,8}, {"RES 3,B",BIT3,B,RES,8}, {"RES 3,C",BIT3,C,RES,8}, {"RES 3,D",BIT3,D,RES,8}, {"RES 3,E",BIT3,E,RES,8}, {"RES 3,H",BIT3,H,RES,8}, {"RES 3,L",BIT3,L,RES,8}, {"RES 3,(HL)",BIT3,IND_HL,RES,16}, {"RES 3,A",BIT3,A,RES,8},
		{"RES 4,B",BIT4,B,RES,8}, {"RES 4,C",BIT4,C,RES,8}, {"RES 4,D",BIT4,D,RES,8}, {"RES 4,E",BIT4,E,RES,8}, {"RES 4,H",BIT4,H,RES,8}, {"RES 4,L",BIT4,L,RES,8}, {"RES 4,(HL)",BIT4,IND_HL,RES,16}, {"RES 4,A",BIT4,A,RES,8}, {"RES 5,B",BIT5,B,RES,8}, {"RES 5,C",BIT5,C,RES,8}, {"RES 5,D",BIT5,D,RES,8}, {"RES 5,E",BIT5,E,RES,8}, {"RES 5,H",BIT5,H,RES,8}, {"RES 5,L",BIT5,L,RES,8}, {"RES 5,(HL)",BIT5,IND_HL,RES,16}, {"RES 5,A",BIT5,A,RES,8},
		{"RES 6,B",BIT6,B,RES,8}, {"RES 6,C",BIT6,C,RES,8}, {"RES 6,D",BIT6,D,RES,8}, {"RES 6,E",BIT6,E,RES,8}, {"RES 6,H",BIT6,H,RES,8}, {"RES 6,L",BIT6,L,RES,8}, {"RES 6,(HL)",BIT6,IND_HL,RES,16}, {"RES 6,A",BIT6,A,RES,8}, {"RES 7,B",BIT7,B,RES,8}, {"RES 7,C",BIT7,C,RES,8}, {"RES 7,D",BIT7,D,RES,8}, {"RES 7,E",BIT7,E,RES,8}, {"RES 7,H",BIT7,H,RES,8}, {"RES 7,L",BIT7,L,RES,8}, {"RES 7,(HL)",BIT7,IND_HL,RES,16}, {"RES 7,A",BIT7,A,RES,8},
		{"SET 0,B",BIT0,B,SET,8}, {"SET 0,C",BIT0,C,SET,8}, {"SET 0,D",BIT0,D,SET,8}, {"SET 0,E",BIT0,E,SET,8}, {"SET 0,H",BIT0,H,SET,8}, {"SET 0,L",BIT0,L,SET,8}, {"SET 0,(HL)",BIT0,IND_HL,SET,16}, {"SET 0,A",BIT0,A,SET,8}, {"SET 1,B",BIT1,B,SET,8}, {"SET 1,C",BIT1,C,SET,8}, {"SET 1,D",BIT1,D,SET,8}, {"SET 1,E",BIT1,E,SET,8}, {"SET 1,H",BIT1,H,SET,8}, {"SET 1,L",BIT1,L,SET,8}, {"SET 1,(HL)",BIT1,IND_HL,SET,16}, {"SET 1,A",BIT1,A,SET,8},
		{"SET 2,B",BIT2,B,SET,8}, {"SET 2,C",BIT2,C,SET,8}, {"SET 2,D",BIT2,D,SET,8}, {"SET 2,E",BIT2,E,SET,8}, {"SET 2,H",BIT2,H,SET,8}, {"SET 2,L",BIT2,L,SET,8}, {"SET 2,(HL)",BIT2,IND_HL,SET,16}, {"SET 2,A",BIT2,A,SET,8}, {"SET 3,B",BIT3,B,SET,8}, {"SET 3,C",BIT3,C,SET,8}, {"SET 3,D",BIT3,D,SET,8}, {"SET 3,E",BIT3,E,SET,8}, {"SET 3,H",BIT3,H,SET,8}, {"SET 3,L",BIT3,L,SET,8}, {"SET 3,(HL)",BIT3,IND_HL,SET,16}, {"SET 3,A",BIT3,A,SET,8},
		{"SET 4,B",BIT4,B,SET,8}, {"SET 4,C",BIT4,C,SET,8}, {"SET 4,D",BIT4,D,SET,8}, {"SET 4,E",BIT4,E,SET,8}, {"SET 4,H",BIT4,H,SET,8}, {"SET 4,L",BIT4,L,SET,8}, {"SET 4,(HL)",BIT4,IND_HL,SET,16}, {"SET 4,A",BIT4,A,SET,8}, {"SET 5,B",BIT5,B,SET,8}, {"SET 5,C",BIT5,C,SET,8}, {"SET 5,D",BIT5,D,SET,8}, {"SET 5,E",BIT5,E,SET,8}, {"SET 5,H",BIT5,H,SET,8}, {"SET 5,L",BIT5,L,SET,8}, {"SET 5,(HL)",BIT5,IND_HL,SET,16}, {"SET 5,A",BIT5,A,SET,8},
		{"SET 6,B",BIT6,B,SET,8}, {"SET 6,C",BIT6,C,SET,8}, {"SET 6,D",BIT6,D,SET,8}, {"SET 6,E",BIT6,E,SET,8}, {"SET 6,H",BIT6,H,SET,8}, {"SET 6,L",BIT6,L,SET,8}, {"SET 6,(HL)",BIT6,IND_HL,SET,16}, {"SET 6,A",BIT6,A,SET,8}, {"SET 7,B",BIT7,B,SET,8}, {"SET 7,C",BIT7,C,SET,8}, {"SET 7,D",BIT7,D,SET,8}, {"SET 7,E",BIT7,E,SET,8}, {"SET 7,H",BIT7,H,SET,8}, {"SET 7,L",BIT7,L,SET,8}, {"SET 7,(HL)",BIT7,IND_HL,SET,16}, {"SET 7,A",BIT7,A,SET,8}
};

u8 PREFIX_CB() {
	u8 op = device.cpu.imm8;
	cb_instr_t ins = cb_instructions[op];
	ins.bit();
	ins.reg();
	ins.op();
	return ins.cycles;
}

const instruction_t INVALID = { "INV",0,0,0 };
static instruction_t instructions[256] = {
	/*                                   x0                             x1                               x2                              x3                                x4                              x5                                 x6                             x7                               x8                           x9                               xA                              xB                              xC                          xD                             xE                        xF */
						  {"NOP",IMP,NOP,4},  {"LD BC,d16",BC_IMM,MOVD,12},   {"LD (BC),A",IND_BC_A,STOS,8},           {"INC BC",BC,INCD,8},             {"INC B", B,INCS, 4},             {"DEC B",B,DECS,4},          {"LD B,d8",B_IMM,MOVS,8},           {"RLCA",IMP,RLCA,4}, {"LD (a16),SP",ADDR_SP,STOD,20}, {"ADD HL,BC",HL_BC,ADD16,8},   {"LD A,(BC)",IND_A_BC,LOAS,8},           {"DEC BC",BC,DECD,8},             {"INC C",C,INCS,4},         {"DEC C",C,DECS,4},      {"LD C,d8",C_IMM,MOVS,8},      {"RRCA",IMP,RRCA,4},
					  {"STOP 0",IMP,STOP,4},  {"LD DE,d16",DE_IMM,MOVD,12},   {"LD (DE),A",IND_DE_A,STOS,8},           {"INC DE",DE,INCD,8},             {"INC D", D,INCS, 4},             {"DEC D",D,DECS,4},          {"LD D,d8",D_IMM,MOVS,8},             {"RLA",IMP,RLA,4},           {" JR r8",IMM8,JR,12}, {"ADD HL,DE",HL_DE,ADD16,8},   {"LD A,(DE)",IND_A_DE,LOAS,8},           {"DEC DE",DE,DECD,8},             {"INC E",E,INCS,4},         {"DEC E",E,DECS,4},      {"LD E,d8",E_IMM,MOVS,8},        {"RRA",IMP,RRA,4},
				   {"JR NZ,r8",IMM8,JRNZ,8},  {"LD HL,d16",HL_IMM,MOVD,12}, {"LD (HL+),A",IND_HLI_A,STOS,8},           {"INC HL",HL,INCD,8},             {"INC H", H,INCS, 4},             {"DEC H",H,DECS,4},          {"LD H,d8",H_IMM,MOVS,8},             {"DAA",IMP,DAA,4},          {"JR Z,r8",IMM8,JRZ,8}, {"ADD HL,HL",HL_HL,ADD16,8}, {"LD A,(HL+)",IND_A_HLI,LOAS,8},           {"DEC HL",HL,DECD,8},             {"INC L",L,INCS,4},         {"DEC L",L,DECS,4},      {"LD L,d8",L_IMM,MOVS,8},        {"CPL",IMP,CPL,4},
				   {"JR NC,r8",IMM8,JRNC,8},  {"LD SP,d16",SP_IMM,MOVD,12}, {"LD (HL-),A",IND_HLD_A,STOS,8},           {"INC SP",SP,INCD,8},   {"INC (HL)",IMP,INC_IND_HL,12}, {"DEC (HL)",IMP,DEC_IND_HL,12}, {"LD (HL),d8",IND_HL_IMM,STOS,12},             {"SCF",IMP,SCF,4},          {"JR C,r8",IMM8,JRC,8}, {"ADD HL,SP",HL_SP,ADD16,8}, {"LD A,(HL-)",IND_A_HLD,LOAS,8},           {"DEC SP",SP,DECD,8},             {"INC A",A,INCS,4},         {"DEC A",A,DECS,4},      {"LD A,d8",A_IMM,MOVS,8},        {"CCF",IMP,CCF,4},
					  {"LD B,B",B_B,MOVS,4},         {"LD B,C",B_C,MOVS,4},           {"LD B,D",B_D,MOVS,4},          {"LD B,E",B_E,MOVS,4},            {"LD B,H",B_H,MOVS,4},          {"LD B,L",B_L,MOVS,4},     {"LD B,(HL)",IND_B_HL,LOAS,8},         {"LD B,A",B_A,MOVS,4},           {"LD C,B",C_B,MOVS,4},       {"LD C,C",C_C,MOVS,4},           {"LD C,D",C_D,MOVS,4},          {"LD C,E",C_E,MOVS,4},          {"LD C,H",C_H,MOVS,4},      {"LD C,L",C_L,MOVS,4}, {"LD C,(HL)",IND_C_HL,LOAS,8},    {"LD C,A",C_A,MOVS,4},
					  {"LD D,B",D_B,MOVS,4},         {"LD D,C",D_C,MOVS,4},           {"LD D,D",D_D,MOVS,4},          {"LD D,E",D_E,MOVS,4},            {"LD D,H",D_H,MOVS,4},          {"LD D,L",D_L,MOVS,4},     {"LD D,(HL)",IND_D_HL,LOAS,8},         {"LD D,A",D_A,MOVS,4},           {"LD E,B",E_B,MOVS,4},       {"LD E,C",E_C,MOVS,4},           {"LD E,D",E_D,MOVS,4},          {"LD E,E",E_E,MOVS,4},          {"LD E,H",E_H,MOVS,4},      {"LD E,L",E_L,MOVS,4}, {"LD E,(HL)",IND_E_HL,LOAS,8},    {"LD E,A",E_A,MOVS,4},
					  {"LD H,B",H_B,MOVS,4},         {"LD H,C",H_C,MOVS,4},           {"LD H,D",H_D,MOVS,4},          {"LD H,E",H_E,MOVS,4},            {"LD H,H",H_H,MOVS,4},          {"LD H,L",H_L,MOVS,4},     {"LD H,(HL)",IND_H_HL,LOAS,8},         {"LD H,A",H_A,MOVS,4},           {"LD L,B",L_B,MOVS,4},       {"LD L,C",L_C,MOVS,4},           {"LD L,D",L_D,MOVS,4},          {"LD L,E",L_E,MOVS,4},          {"LD L,H",L_H,MOVS,4},      {"LD L,L",L_L,MOVS,4}, {"LD L,(HL)",IND_L_HL,LOAS,8},    {"LD L,A",L_A,MOVS,4},
			  {"LD (HL),B",IND_HL_B,STOS,8}, {"LD (HL),C",IND_HL_C,STOS,8},   {"LD (HL),D",IND_HL_D,STOS,8},  {"LD (HL),E",IND_HL_E,STOS,8},    {"LD (HL),H",IND_HL_H,STOS,8},  {"LD (HL),L",IND_HL_L,STOS,8},               {"HALT",IMP,HALT,4}, {"LD (HL),A",IND_HL_A,STOS,8},           {"LD A,B",A_B,MOVS,4},       {"LD A,C",A_C,MOVS,4},           {"LD A,D",A_D,MOVS,4},          {"LD A,E",A_E,MOVS,4},          {"LD A,H",A_H,MOVS,4},      {"LD A,L",A_L,MOVS,4}, {"LD A,(HL)",IND_A_HL,LOAS,8},    {"LD A,A",A_A,MOVS,4},
						{"ADD A,B",B,ADD,4},           {"ADD A,C",C,ADD,4},             {"ADD A,D",D,ADD,4},            {"ADD A,E",E,ADD,4},              {"ADD A,H",H,ADD,4},            {"ADD A,L",L,ADD,4},       {"ADD A,(HL)",IND_HL,ADD,8},           {"ADD A,A",A,ADD,4},             {"ADC A,B",B,ADC,4},         {"ADC A,C",C,ADC,4},             {"ADC A,D",D,ADC,4},            {"ADC A,E",E,ADC,4},            {"ADC A,H",H,ADC,4},        {"ADC A,L",L,ADC,4},      {"ADC A,(HL)",IND,ADC,8},      {"ADC A,A",A,ADC,4},
						  {"SUB B",B,SUB,4},             {"SUB C",C,SUB,4},               {"SUB D",D,SUB,4},              {"SUB E",E,SUB,4},                {"SUB H",H,SUB,4},              {"SUB L",L,SUB,4},         {"SUB (HL)",IND_HL,SUB,8},             {"SUB A",A,SUB,4},               {"SBC B",B,SBC,4},           {"SBC C",C,SBC,4},               {"SBC D",D,SBC,4},              {"SBC E",E,SBC,4},              {"SBC H",H,SBC,4},          {"SBC L",L,SBC,4},        {"SBC (HL)",IND,SBC,8},        {"SBC A",A,SBC,4},
						  {"AND B",B,AND,4},             {"AND C",C,AND,4},               {"AND D",D,AND,4},              {"AND E",E,AND,4},                {"AND H",H,AND,4},              {"AND L",L,AND,4},         {"AND (HL)",IND_HL,AND,8},             {"AND A",A,AND,4},               {"XOR B",B,XOR,4},           {"XOR C",C,XOR,4},               {"XOR D",D,XOR,4},              {"XOR E",E,XOR,4},              {"XOR H",H,XOR,4},          {"XOR L",L,XOR,4},        {"XOR (HL)",IND,XOR,8},        {"XOR A",A,XOR,4},
							{"OR B",B,OR,4},               {"OR C",C,OR,4},                 {"OR D",D,OR,4},                {"OR E",E,OR,4},                  {"OR H",H,OR,4},                {"OR L",L,OR,4},           {"OR (HL)",IND_HL,OR,8},               {"OR A",A,OR,4},                 {"CP B",B,CP,4},             {"CP C",C,CP,4},                 {"CP D",D,CP,4},                {"CP E",E,CP,4},                {"CP H",H,CP,4},            {"CP L",L,CP,4},          {"CP (HL)",IND,CP,8},          {"CP A",A,CP,4},
					 {"RET NZ",IMP,RETNZ,8},          {"POP BC",BC,POP,12},  {"JP NZ,a16",ADDR_IMM,JPNZ,12},      {"JP a16",ADDR_IMM,JP,16}, {"CALL NZ,a16",IMM16,CALL_NZ,12},         {"PUSH BC",BC,PUSH,16},           {"ADD A,d8",IMM8,ADD,8},      {"RST 00H",IMP,RST00,16},            {"RET Z",IMP,RETZ,8},          {"RET",IMP,RET,16},    {"JP Z,a16",ADDR_IMM,JPZ,12}, {"PREFIX CB",IMM8,PREFIX_CB,0}, {"CALL Z,a16",IMM16,CALL_Z,12}, {"CALL a16",IMM16,CALL,24},       {"ADC A,d8",IMM8,ADC,8}, {"RST 08H",IMP,RST08,16},
					 {"RET NC",IMP,RETNC,8},          {"POP DE",DE,POP,12},  {"JP NC,a16",ADDR_IMM,JPNC,12},                        INVALID, {"CALL NZ,a16",IMM16,CALL_NC,12},         {"PUSH DE",DE,PUSH,16},             {"SUB d8",IMM8,SUB,8},      {"RST 10H",IMP,RST10,16},            {"RET C",IMP,RETC,8},        {"RETI",IMP,RETI,16},    {"JP C,a16",ADDR_IMM,JPC,12},                        INVALID, {"CALL C,a16",IMM16,CALL_C,12},                    INVALID,       {"SBC A,d8",IMM8,SBC,8}, {"RST 18H",IMP,RST18,16},
		   {"LDH (a8),A",ADDR_HI_A,STOS,12},          {"POP HL",HL,POP,12},     {"LD (C),A",IND_C_A,STOS,8},                        INVALID,                          INVALID,         {"PUSH HL",HL,PUSH,16},             {"AND d8",IMM8,AND,8},      {"RST 20H",IMP,RST20,16},     {"ADD SP,r8",IMM8,ADDSP,16},    {"JP (HL)",ADDR_HL,JP,4},   {"LD (a16),A",ADDR_A,STOS,16},                        INVALID,                        INVALID,                    INVALID,       {"XOR A,d8",IMM8,XOR,8}, {"RST 28H",IMP,RST28,16},
		   {"LDH A,(a8)",ADDR_A_HI,LOAS,12},          {"POP AF",AF,POP,12},     {"LD A,(C)",IND_C_A,LOAS,8},                {"DI",IMP,DI,4},                          INVALID,         {"PUSH AF",AF,PUSH,16},               {"OR d8",IMM8,OR,8},      {"RST 30H",IMP,RST30,16},  {"LD HL,SP+r8",IMM8,LDHLSP,12},   {"LD SP,HL",SP_HL,MOVD,8},   {"LD A,(a16)",ADDR_A,LOAS,16},                {"EI",IMP,EI,4},                        INVALID,                    INVALID,         {"CP A,d8",IMM8,CP,8}, {"RST 38H",IMP,RST38,16}
};

instruction_t cpu_decode(u8 opcode) {
	return instructions[opcode];
}

void cpu_handle_interrupts() {
	if (device.cpu.ime) {
		// check for interrupts
		if (device.ie & device.io[IO_IF] & INT_MASK) {
			u16 int_addr = 0x000;
			if (device.ie & device.io[IO_IF] & INT_VBLANK) {
				int_addr = 0x0040;
				device.io[IO_IF] = device.io[IO_IF] & ~INT_VBLANK;
			}
			else if (device.ie & device.io[IO_IF] & INT_LCDC) {
				int_addr = 0x0048;
				device.io[IO_IF] = device.io[IO_IF] & ~INT_LCDC;
			}
			else if (device.ie & device.io[IO_IF] & INT_TIMER) {
				int_addr = 0x0050;
				device.io[IO_IF] = device.io[IO_IF] & ~INT_TIMER;
			}
			else if (device.ie & device.io[IO_IF] & INT_SERIAL_TRANSFER) {
				int_addr = 0x0058;
				device.io[IO_IF] = device.io[IO_IF] & ~INT_SERIAL_TRANSFER;
			}
			else if (device.ie & device.io[IO_IF] & INT_P1) {
				int_addr = 0x0060;
				device.io[IO_IF] = device.io[IO_IF] & ~INT_P1;
			}
			PUSH_PC();
			device.cpu.registers.pc = int_addr;
			device.cpu.disable_interrupt = true;
			device.cpu.halted = false;
		}
	}
	if (device.cpu.disable_interrupt) {
		device.cpu.disable_interrupt = false;
		device.cpu.ime = 0;
	}
	if (device.cpu.enable_interrupt) {
		device.cpu.enable_interrupt = false;
		device.cpu.ime = 1;
	}
}

void cpu_exec(instruction_t ins) {
	ins.mode();
	u8 additional_cycles = ins.op();
	device.cpu.cycles = ins.cycles + additional_cycles;
}

void cpu_clock() {
	if (device.cpu.cycles-- == 0) {
		cpu_handle_interrupts();

		if (!device.cpu.halted) {
			device.ins_ptr = device.cpu.registers.pc;
#if _ENABLE_DUMP
			dump_cpu_state();
#endif
			if (device.ins_ptr == 0xc31a) {
				int x = 0;
			}

			u8 op = cpu_fetch();
			instruction_t ins = instructions[op];
			cpu_exec(ins);
			int _vs_studio_sucks = 0;
		}
	}
}