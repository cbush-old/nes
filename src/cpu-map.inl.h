switch (last_op)
{
        case 0x00: BRK(); break;
    case 0x01: ORA<&CPU::IDX>(); break;
    case 0x02: BAD_OP(); break;
    case 0x03: unofficial<&CPU::rmw<&CPU::IDX, &CPU::ASL>, &CPU::ORA<&CPU::IDX> >(); break; //SLO
    case 0x04: NOP<&CPU::IMM>(); break;
    case 0x05: ORA<&CPU::ZPG>(); break;
    case 0x06: rmw<&CPU::ZPG, &CPU::ASL>(); break;
    case 0x07: unofficial<&CPU::rmw<&CPU::ZPG, &CPU::ASL>, &CPU::ORA<&CPU::ZPG> >(); break; //SLO
    case 0x08: stack_push<&CPU::ProcStatus>(); break;
    case 0x09: ORA<&CPU::IMM>(); break;
    case 0x0A: rmw<&CPU::ACC, &CPU::ASL>(); break;
    case 0x0B: unofficial<&CPU::AND<&CPU::IMM>, &CPU::copyflag<N_FLAG, C_FLAG>>(); break; //ANC
    case 0x0C: NOP<&CPU::ABS>(); break;
    case 0x0D: ORA<&CPU::ABS>(); break;
    case 0x0E: rmw<&CPU::ABS, &CPU::ASL>(); break;
    case 0x0F: unofficial<&CPU::rmw<&CPU::ABS, &CPU::ASL>, &CPU::ORA<&CPU::ABS>>(); break; //SLO
    case 0x10: branch<&CPU::if_clear<N_FLAG> >(); break;
    case 0x11: ORA<&CPU::IDY_pgx>(); break;
    case 0x12: BAD_OP(); break;
    case 0x13: unofficial<&CPU::rmw<&CPU::IDY, &CPU::ASL>, &CPU::ORA<&CPU::IDY> >(); break; //SLO
    case 0x14: NOP<&CPU::ZPX>(); break;
    case 0x15: ORA<&CPU::ZPX>(); break;
    case 0x16: rmw<&CPU::ZPX, &CPU::ASL>(); break;
    case 0x17: unofficial<&CPU::rmw<&CPU::ZPX, &CPU::ASL>, &CPU::ORA<&CPU::ZPX> >(); break; //SLO
    case 0x18: clear<C_FLAG>(); break;
    case 0x19: ORA<&CPU::ABY_pgx>(); break;
    case 0x1A: NOP(); break;
    case 0x1B: unofficial<&CPU::rmw<&CPU::ABY, &CPU::ASL>, &CPU::ORA<&CPU::ABY>>(); break; //SLO
    case 0x1C: NOP<&CPU::ABX_pgx>(); break;
    case 0x1D: ORA<&CPU::ABX_pgx>(); break;
    case 0x1E: rmw<&CPU::ABX, &CPU::ASL>(); break;
    case 0x1F: unofficial<&CPU::rmw<&CPU::ABX, &CPU::ASL>, &CPU::ORA<&CPU::ABX>>(); break; //SLO
    case 0x20: JSR(); break;
    case 0x21: AND<&CPU::IDX>(); break;
    case 0x22: BAD_OP(); break;
    case 0x23: unofficial<&CPU::rmw<&CPU::IDX, &CPU::ROL>, &CPU::AND<&CPU::IDX> >(); break; //RLA
    case 0x24: BIT<&CPU::ZPG>(); break;
    case 0x25: AND<&CPU::ZPG>(); break;
    case 0x26: rmw<&CPU::ZPG, &CPU::ROL>(); break;
    case 0x27: unofficial<&CPU::rmw<&CPU::ZPG, &CPU::ROL>, &CPU::AND<&CPU::ZPG> >(); break; //RLA
    case 0x28: stack_pull<&CPU::ProcStatus>(); break;
    case 0x29: AND<&CPU::IMM>(); break;
    case 0x2A: rmw<&CPU::ACC, &CPU::ROL>(); break;
    case 0x2B: unofficial<&CPU::AND<&CPU::IMM>, &CPU::copyflag<N_FLAG, C_FLAG>>(); break; //ANC
    case 0x2C: BIT<&CPU::ABS>(); break;
    case 0x2D: AND<&CPU::ABS>(); break;
    case 0x2E: rmw<&CPU::ABS, &CPU::ROL>(); break;
    case 0x2F: unofficial<&CPU::rmw<&CPU::ABS, &CPU::ROL>, &CPU::AND<&CPU::ABS>>(); break; //RLA
    case 0x30: branch<&CPU::if_set<N_FLAG> >(); break;
    case 0x31: AND<&CPU::IDY_pgx>(); break;
    case 0x32: BAD_OP(); break;
    case 0x33: unofficial<&CPU::rmw<&CPU::IDY, &CPU::ROL>, &CPU::AND<&CPU::IDY> >(); break; //RLA
    case 0x34: NOP<&CPU::ZPX>(); break;
    case 0x35: AND<&CPU::ZPX>(); break;
    case 0x36: rmw<&CPU::ZPX, &CPU::ROL>(); break;
    case 0x37: unofficial<&CPU::rmw<&CPU::ZPX, &CPU::ROL>, &CPU::AND<&CPU::ZPX> >(); break; //RLA
    case 0x38: set<C_FLAG>(); break;
    case 0x39: AND<&CPU::ABY_pgx>(); break;
    case 0x3A: NOP(); break;
    case 0x3B: unofficial<&CPU::rmw<&CPU::ABY, &CPU::ROL>, &CPU::AND<&CPU::ABY>>(); break; //RLA
    case 0x3C: NOP<&CPU::ABX_pgx>(); break;
    case 0x3D: AND<&CPU::ABX_pgx>(); break;
    case 0x3E: rmw<&CPU::ABX, &CPU::ROL>(); break;
    case 0x3F: unofficial<&CPU::rmw<&CPU::ABX, &CPU::ROL>, &CPU::AND<&CPU::ABX>>(); break; //RLA
    case 0x40: RTI(); break;
    case 0x41: EOR<&CPU::IDX>(); break;
    case 0x42: BAD_OP(); break;
    case 0x43: unofficial<&CPU::rmw<&CPU::IDX, &CPU::LSR>, &CPU::EOR<&CPU::IDX> >(); break; //SRE
    case 0x44: NOP<&CPU::IMM>(); break;
    case 0x45: EOR<&CPU::ZPG>(); break;
    case 0x46: rmw<&CPU::ZPG, &CPU::LSR>(); break;
    case 0x47: unofficial<&CPU::rmw<&CPU::ZPG, &CPU::LSR>, &CPU::EOR<&CPU::ZPG> >(); break; //SRE
    case 0x48: stack_push<&CPU::Accumulator>(); break;
    case 0x49: EOR<&CPU::IMM>(); break;
    case 0x4A: rmw<&CPU::ACC, &CPU::LSR>(); break;
    case 0x4B: unofficial<&CPU::AND<&CPU::IMM>, &CPU::rmw<&CPU::ACC, &CPU::LSR>>(); break; // ALR
    case 0x4C: jump<&CPU::ABS>(); break;
    case 0x4D: EOR<&CPU::ABS>(); break;
    case 0x4E: rmw<&CPU::ABS, &CPU::LSR>(); break;
    case 0x4F: unofficial<&CPU::rmw<&CPU::ABS, &CPU::LSR>, &CPU::EOR<&CPU::ABS>>(); break; //SRE
    case 0x50: branch<&CPU::if_clear<V_FLAG> >(); break;
    case 0x51: EOR<&CPU::IDY_pgx>(); break;
    case 0x52: BAD_OP(); break;
    case 0x53: unofficial<&CPU::rmw<&CPU::IDY, &CPU::LSR>, &CPU::EOR<&CPU::IDY> >(); break; //SRE
    case 0x54: NOP<&CPU::ZPX>(); break;
    case 0x55: EOR<&CPU::ZPX>(); break;
    case 0x56: rmw<&CPU::ZPX, &CPU::LSR>(); break;
    case 0x57: unofficial<&CPU::rmw<&CPU::ZPX, &CPU::LSR>, &CPU::EOR<&CPU::ZPX> >(); break; //SRE
    case 0x58: clear<I_FLAG>(); break;
    case 0x59: EOR<&CPU::ABY_pgx>(); break;
    case 0x5A: NOP(); break;
    case 0x5B: unofficial<&CPU::rmw<&CPU::ABY, &CPU::LSR>, &CPU::EOR<&CPU::ABY>>(); break; //SRE
    case 0x5C: NOP<&CPU::ABX_pgx>(); break;
    case 0x5D: EOR<&CPU::ABX_pgx>(); break;
    case 0x5E: rmw<&CPU::ABX, &CPU::LSR>(); break;
    case 0x5F: unofficial<&CPU::rmw<&CPU::ABX, &CPU::LSR>, &CPU::EOR<&CPU::ABX>>(); break; //SRE
    case 0x60: RTS(); break;
    case 0x61: ADC<&CPU::IDX>(); break;
    case 0x62: BAD_OP(); break;
    case 0x63: unofficial<&CPU::rmw<&CPU::IDX, &CPU::ROR>, &CPU::ADC<&CPU::IDX> >(); break; //RRA
    case 0x64: NOP<&CPU::IMM>(); break;
    case 0x65: ADC<&CPU::ZPG>(); break;
    case 0x66: rmw<&CPU::ZPG, &CPU::ROR>(); break;
    case 0x67: unofficial<&CPU::rmw<&CPU::ZPG, &CPU::ROR>, &CPU::ADC<&CPU::ZPG> >(); break; //RRA
    case 0x68: stack_pull<&CPU::Accumulator>(); break;
    case 0x69: ADC<&CPU::IMM>(); break;
    case 0x6A: rmw<&CPU::ACC, &CPU::ROR>(); break;
    case 0x6B: unofficial<&CPU::AND<&CPU::IMM>, &CPU::rmw<&CPU::ACC, &CPU::ROR>>(); break; //ARR
    case 0x6C: jump<&CPU::IND>(); break;
    case 0x6D: ADC<&CPU::ABS>(); break;
    case 0x6E: rmw<&CPU::ABS, &CPU::ROR>(); break;
    case 0x6F: unofficial<&CPU::rmw<&CPU::ABS, &CPU::ROR>, &CPU::ADC<&CPU::ABS>>(); break; //RRA
    case 0x70: branch<&CPU::if_set<V_FLAG> >(); break;
    case 0x71: ADC<&CPU::IDY_pgx>(); break;
    case 0x72: BAD_OP(); break;
    case 0x73: unofficial<&CPU::rmw<&CPU::IDY, &CPU::ROR>, &CPU::ADC<&CPU::IDY> >(); break; //RRA
    case 0x74: NOP<&CPU::ZPX>(); break;
    case 0x75: ADC<&CPU::ZPX>(); break;
    case 0x76: rmw<&CPU::ZPX, &CPU::ROR>(); break;
    case 0x77: unofficial<&CPU::rmw<&CPU::ZPX, &CPU::ROR>, &CPU::ADC<&CPU::ZPX> >(); break; //RRA
    case 0x78: set<I_FLAG>(); break;
    case 0x79: ADC<&CPU::ABY_pgx>(); break;
    case 0x7A: NOP(); break;
    case 0x7B: unofficial<&CPU::rmw<&CPU::ABY, &CPU::ROR>, &CPU::ADC<&CPU::ABY>>(); break; //RRA
    case 0x7C: NOP<&CPU::ABX_pgx>(); break;
    case 0x7D: ADC<&CPU::ABX_pgx>(); break;
    case 0x7E: rmw<&CPU::ABX, &CPU::ROR>(); break;
    case 0x7F: unofficial<&CPU::rmw<&CPU::ABX, &CPU::ROR>, &CPU::ADC<&CPU::ABX>>(); break; //RRA
    case 0x80: NOP<&CPU::ZPG>(); break;
    case 0x81: store<&CPU::Accumulator, &CPU::IDX>(); break;
    case 0x82: NOP<&CPU::IMM>(); break;
    case 0x83: store<&CPU::AX, &CPU::IDX>(); break; //SAX
    case 0x84: store<&CPU::IndexRegY, &CPU::ZPG>(); break;
    case 0x85: store<&CPU::Accumulator, &CPU::ZPG>(); break;
    case 0x86: store<&CPU::IndexRegX, &CPU::ZPG>(); break;
    case 0x87: store<&CPU::AX, &CPU::ZPG>(); break; //SAX
    case 0x88: rmw<&CPU::Y__, &CPU::DEC>(); break;
    case 0x89: NOP<&CPU::IMM>(); break;
    case 0x8A: transfer<&CPU::IndexRegX, &CPU::Accumulator>(); break;
    case 0x8B: NOP<&CPU::IMM>(); break; //XAA
    case 0x8C: store<&CPU::IndexRegY, &CPU::ABS>(); break;
    case 0x8D: store<&CPU::Accumulator, &CPU::ABS>(); break;
    case 0x8E: store<&CPU::IndexRegX, &CPU::ABS>(); break;
    case 0x8F: store<&CPU::AX, &CPU::ABS>(); break; //SAX
    case 0x90: branch<&CPU::if_clear<C_FLAG> >(); break;
    case 0x91: store<&CPU::Accumulator, &CPU::IDY>(); break;
    case 0x92: BAD_OP(); break;
    case 0x93: NOP<&CPU::IDY>(); break; //AHX<&CPU::IDY>
    case 0x94: store<&CPU::IndexRegY, &CPU::ZPX>(); break;
    case 0x95: store<&CPU::Accumulator, &CPU::ZPX>(); break;
    case 0x96: store<&CPU::IndexRegX, &CPU::ZPY>(); break;
    case 0x97: store<&CPU::AX, &CPU::ZPY>(); break; //SAX
    case 0x98: transfer<&CPU::IndexRegY, &CPU::Accumulator>(); break;
    case 0x99: store<&CPU::Accumulator, &CPU::ABY>(); break;
    case 0x9A: transfer<&CPU::IndexRegX, &CPU::StackPointer>(); break;
    case 0x9B: NOP<&CPU::ABY>(); break; //TAS<&CPU::ABY>
    case 0x9C: NOP<&CPU::ABX>(); break; //SHY<&CPU::ABX>
    case 0x9D: store<&CPU::Accumulator, &CPU::ABX>(); break;
    case 0x9E: NOP<&CPU::ABY>(); break; //SHX<&CPU::ABY>
    case 0x9F: NOP<&CPU::ABY>(); break; //AHX<&CPU::ABY>
    case 0xA0: load<&CPU::IndexRegY, &CPU::IMM>(); break;
    case 0xA1: load<&CPU::Accumulator, &CPU::IDX>(); break;
    case 0xA2: load<&CPU::IndexRegX, &CPU::IMM>(); break;
    case 0xA3: unofficial<&CPU::load<&CPU::Accumulator, &CPU::IDX>, &CPU::transfer<&CPU::Accumulator, &CPU::IndexRegX>>(); break; //LAX
    case 0xA4: load<&CPU::IndexRegY, &CPU::ZPG>(); break;
    case 0xA5: load<&CPU::Accumulator, &CPU::ZPG>(); break;
    case 0xA6: load<&CPU::IndexRegX, &CPU::ZPG>(); break;
    case 0xA7: unofficial<&CPU::load<&CPU::Accumulator, &CPU::ZPG>, &CPU::transfer<&CPU::Accumulator, &CPU::IndexRegX>>(); break; //LAX
    case 0xA8: transfer<&CPU::Accumulator, &CPU::IndexRegY>(); break;
    case 0xA9: load<&CPU::Accumulator, &CPU::IMM>(); break;
    case 0xAA: transfer<&CPU::Accumulator, &CPU::IndexRegX>(); break;
    case 0xAB: unofficial<&CPU::load<&CPU::Accumulator, &CPU::IMM>, &CPU::transfer<&CPU::Accumulator, &CPU::IndexRegX>>(); break; //LAX
    case 0xAC: load<&CPU::IndexRegY, &CPU::ABS>(); break;
    case 0xAD: load<&CPU::Accumulator, &CPU::ABS>(); break;
    case 0xAE: load<&CPU::IndexRegX, &CPU::ABS>(); break;
    case 0xAF: unofficial<&CPU::load<&CPU::Accumulator, &CPU::ABS>, &CPU::transfer<&CPU::Accumulator, &CPU::IndexRegX>>(); break; //LAX
    case 0xB0: branch<&CPU::if_set<C_FLAG> >(); break;
    case 0xB1: load<&CPU::Accumulator, &CPU::IDY_pgx>(); break;
    case 0xB2: BAD_OP(); break;
    case 0xB3: unofficial<&CPU::load<&CPU::Accumulator, &CPU::IDY_pgx>, &CPU::transfer<&CPU::Accumulator, &CPU::IndexRegX>>(); break; //LAX
    case 0xB4: load<&CPU::IndexRegY, &CPU::ZPX>(); break;
    case 0xB5: load<&CPU::Accumulator, &CPU::ZPX>(); break;
    case 0xB6: load<&CPU::IndexRegX, &CPU::ZPY>(); break;
    case 0xB7: unofficial<&CPU::load<&CPU::Accumulator, &CPU::ZPY>, &CPU::transfer<&CPU::Accumulator, &CPU::IndexRegX>>(); break; //LAX
    case 0xB8: clear<V_FLAG>(); break;
    case 0xB9: load<&CPU::Accumulator, &CPU::ABY_pgx>(); break;
    case 0xBA: transfer<&CPU::StackPointer, &CPU::IndexRegX>(); break;
    case 0xBB: NOP<&CPU::ABY>(); break; //LAS ay
    case 0xBC: load<&CPU::IndexRegY, &CPU::ABX_pgx>(); break;
    case 0xBD: load<&CPU::Accumulator, &CPU::ABX_pgx>(); break;
    case 0xBE: load<&CPU::IndexRegX, &CPU::ABY_pgx>(); break;
    case 0xBF: unofficial<&CPU::load<&CPU::Accumulator, &CPU::ABY_pgx>, &CPU::transfer<&CPU::Accumulator, &CPU::IndexRegX>>(); break; //LAX
    case 0xC0: compare<&CPU::IndexRegY, &CPU::IMM>(); break;
    case 0xC1: compare<&CPU::Accumulator, &CPU::IDX>(); break;
    case 0xC2: NOP<&CPU::IMM>(); break;
    case 0xC3: unofficial<&CPU::rmw<&CPU::IDX, &CPU::DEC>, &CPU::compare<&CPU::Accumulator, &CPU::IDX>>(); break; //DCP
    case 0xC4: compare<&CPU::IndexRegY, &CPU::ZPG>(); break;
    case 0xC5: compare<&CPU::Accumulator, &CPU::ZPG>(); break;
    case 0xC6: rmw<&CPU::ZPG, &CPU::DEC>(); break;
    case 0xC7: unofficial<&CPU::rmw<&CPU::ZPG, &CPU::DEC>, &CPU::compare<&CPU::Accumulator, &CPU::ZPG> >(); break; //DCP
    case 0xC8: rmw<&CPU::Y__, &CPU::INC>(); break;
    case 0xC9: compare<&CPU::Accumulator, &CPU::IMM>(); break;
    case 0xCA: rmw<&CPU::X__, &CPU::DEC>(); break;
    case 0xCB: transfer<&CPU::AX, &CPU::Accumulator>(); break; //AXS
    case 0xCC: compare<&CPU::IndexRegY, &CPU::ABS>(); break;
    case 0xCD: compare<&CPU::Accumulator, &CPU::ABS>(); break;
    case 0xCE: rmw<&CPU::ABS, &CPU::DEC>(); break;
    case 0xCF: unofficial<&CPU::rmw<&CPU::ABS, &CPU::DEC>, &CPU::compare<&CPU::Accumulator, &CPU::ABS>>(); break; //DCP
    case 0xD0: branch<&CPU::if_clear<Z_FLAG> >(); break;
    case 0xD1: compare<&CPU::Accumulator, &CPU::IDY_pgx>(); break;
    case 0xD2: BAD_OP(); break;
    case 0xD3: unofficial<&CPU::rmw<&CPU::IDY, &CPU::DEC>, &CPU::compare<&CPU::Accumulator, &CPU::IDY>>(); break; //DCP
    case 0xD4: NOP<&CPU::ZPX>(); break;
    case 0xD5: compare<&CPU::Accumulator, &CPU::ZPX>(); break;
    case 0xD6: rmw<&CPU::ZPX, &CPU::DEC>(); break;
    case 0xD7: unofficial<&CPU::rmw<&CPU::ZPX, &CPU::DEC>, &CPU::compare<&CPU::Accumulator, &CPU::ZPX>>(); break; //DCP
    case 0xD8: clear<D_FLAG>(); break;
    case 0xD9: compare<&CPU::Accumulator, &CPU::ABY_pgx>(); break;
    case 0xDA: NOP(); break;
    case 0xDB: unofficial<&CPU::rmw<&CPU::ABY, &CPU::DEC>, &CPU::compare<&CPU::Accumulator, &CPU::ABY>>(); break; //DCP
    case 0xDC: NOP<&CPU::ABX_pgx>(); break;
    case 0xDD: compare<&CPU::Accumulator, &CPU::ABX_pgx>(); break;
    case 0xDE: rmw<&CPU::ABX, &CPU::DEC>(); break;
    case 0xDF: unofficial<&CPU::rmw<&CPU::ABX, &CPU::DEC>, &CPU::compare<&CPU::Accumulator, &CPU::ABX>>(); break; //DCP
    case 0xE0: compare<&CPU::IndexRegX, &CPU::IMM>(); break;
    case 0xE1: SBC<&CPU::IDX>(); break;
    case 0xE2: NOP<&CPU::IMM>(); break;
    case 0xE3: unofficial<&CPU::rmw<&CPU::IDX, &CPU::INC>, &CPU::SBC<&CPU::IDX> >(); break; //ISB(ISC)
    case 0xE4: compare<&CPU::IndexRegX, &CPU::ZPG>(); break;
    case 0xE5: SBC<&CPU::ZPG>(); break;
    case 0xE6: rmw<&CPU::ZPG, &CPU::INC>(); break;
    case 0xE7: unofficial<&CPU::rmw<&CPU::ZPG, &CPU::INC>, &CPU::SBC<&CPU::ZPG> >(); break; //ISB(ISC)
    case 0xE8: rmw<&CPU::X__, &CPU::INC>(); break;
    case 0xE9: SBC<&CPU::IMM>(); break;
    case 0xEA: NOP(); break;
    case 0xEB: SBC<&CPU::IMM>(); break;
    case 0xEC: compare<&CPU::IndexRegX, &CPU::ABS>(); break;
    case 0xED: SBC<&CPU::ABS>(); break;
    case 0xEE: rmw<&CPU::ABS, &CPU::INC>(); break;
    case 0xEF: unofficial<&CPU::rmw<&CPU::ABS, &CPU::INC>, &CPU::SBC<&CPU::ABS>>(); break; //ISB(ISC)
    case 0xF0: branch<&CPU::if_set<Z_FLAG> >(); break;
    case 0xF1: SBC<&CPU::IDY_pgx>(); break;
    case 0xF2: BAD_OP(); break;
    case 0xF3: unofficial<&CPU::rmw<&CPU::IDY, &CPU::INC>, &CPU::SBC<&CPU::IDY>>(); break; //ISB(ISC)
    case 0xF4: NOP<&CPU::ZPX>(); break;
    case 0xF5: SBC<&CPU::ZPX>(); break;
    case 0xF6: rmw<&CPU::ZPX, &CPU::INC>(); break;
    case 0xF7: unofficial<&CPU::rmw<&CPU::ZPX, &CPU::INC>, &CPU::SBC<&CPU::ZPX> >(); break; //ISB(ISC)
    case 0xF8: set<D_FLAG>(); break;
    case 0xF9: SBC<&CPU::ABY_pgx>(); break;
    case 0xFA: NOP(); break;
    case 0xFB: unofficial<&CPU::rmw<&CPU::ABY, &CPU::INC>, &CPU::SBC<&CPU::ABY>>(); break; //ISB(ISC)
    case 0xFC: NOP<&CPU::ABX_pgx>(); break;
    case 0xFD: SBC<&CPU::ABX_pgx>(); break;
    case 0xFE: rmw<&CPU::ABX, &CPU::INC>(); break;
    case 0xFF: unofficial<&CPU::rmw<&CPU::ABX, &CPU::INC>, &CPU::SBC<&CPU::ABX>>(); break; //ISB(ISC)
}
