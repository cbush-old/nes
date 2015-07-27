#include "cpu.h"

// cutting down on the number of times
// &CPU:: would otherwise appear
#define ACC &CPU::ACC
#define X__ &CPU::X__
#define Y__ &CPU::Y__
#define IMM &CPU::IMM
#define ZPG &CPU::ZPG
#define ZPX &CPU::ZPX
#define ZPY &CPU::ZPY
#define ABS &CPU::ABS
#define ABX &CPU::ABX
#define ABY &CPU::ABY
#define IDX &CPU::IDX
#define IDY &CPU::IDY
#define IND &CPU::IND
#define BAD_OP &CPU::BAD_OP
#define JSR &CPU::JSR
#define BRK &CPU::BRK
#define RTS &CPU::RTS
#define RTI &CPU::RTI
#define NOP &CPU::NOP
#define Accumulator &CPU::Accumulator
#define IndexRegX &CPU::IndexRegX
#define IndexRegY &CPU::IndexRegY
#define StackPointer &CPU::StackPointer
#define ProcStatus &CPU::ProcStatus
#define Accumulator &CPU::Accumulator
#define IndexRegX &CPU::IndexRegX
#define IndexRegY &CPU::IndexRegY
#define StackPointer &CPU::StackPointer
#define ProcStatus &CPU::ProcStatus
#define rmw &CPU::rmw
#define AX &CPU::AX
#define ABY_pgx &CPU::ABY_pgx
#define ABX_pgx &CPU::ABX_pgx
#define IDY_pgx &CPU::IDY_pgx
#define load &CPU::load
#define store &CPU::store
#define transfer &CPU::transfer
#define stack_push &CPU::stack_push
#define stack_pull &CPU::stack_pull
#define set &CPU::set
#define clear &CPU::clear
#define if_clear &CPU::if_clear
#define if_set &CPU::if_set
#define AND &CPU::AND
#define EOR &CPU::EOR
#define ORA &CPU::ORA
#define BIT &CPU::BIT
#define compare &CPU::compare
#define ADC &CPU::ADC
#define SBC &CPU::SBC
#define INC &CPU::INC
#define DEC &CPU::DEC
#define ASL &CPU::ASL
#define LSR &CPU::LSR
#define ROL &CPU::ROL
#define ROR &CPU::ROR
#define jump &CPU::jump
#define branch &CPU::branch
#define copyflag &CPU::copyflag
#define unofficial &CPU::unofficial
#define N_FLAG CPU::N_FLAG
#define V_FLAG CPU::V_FLAG
#define D_FLAG CPU::D_FLAG
#define I_FLAG CPU::I_FLAG
#define Z_FLAG CPU::Z_FLAG
#define C_FLAG CPU::C_FLAG

// a complete dispatch table
const CPU::op CPU::ops[256] {
/* 0x00 */ BRK,
/* 0x01 */ ORA<IDX>,
/* 0x02 */ BAD_OP,
/* 0x03 */ unofficial<rmw<IDX, ASL>,ORA<IDX>>,//SLO
/* 0x04 */ NOP<IMM>,
/* 0x05 */ ORA<ZPG>,
/* 0x06 */ rmw<ZPG, ASL>,
/* 0x07 */ unofficial<rmw<ZPG, ASL>,ORA<ZPG>>,//SLO
/* 0x08 */ stack_push<ProcStatus>,	
/* 0x09 */ ORA<IMM>,
/* 0x0A */ rmw<ACC, ASL>,
/* 0x0B */ unofficial<AND<IMM>,copyflag<N_FLAG,C_FLAG>,0>,//ANC
/* 0x0C */ NOP<ABS>,
/* 0x0D */ ORA<ABS>,
/* 0x0E */ rmw<ABS, ASL>,
/* 0x0F */ unofficial<rmw<ABS, ASL>,ORA<ABS>,2>,//SLO
/* 0x10 */ branch<if_clear<N_FLAG>>,
/* 0x11 */ ORA<IDY_pgx>,
/* 0x12 */ BAD_OP,
/* 0x13 */ unofficial<rmw<IDY, ASL>,ORA<IDY>>,//SLO
/* 0x14 */ NOP<ZPX>,
/* 0x15 */ ORA<ZPX>,
/* 0x16 */ rmw<ZPX, ASL>,
/* 0x17 */ unofficial<rmw<ZPX, ASL>,ORA<ZPX>>,//SLO
/* 0x18 */ clear<C_FLAG>,
/* 0x19 */ ORA<ABY_pgx>,
/* 0x1A */ NOP,
/* 0x1B */ unofficial<rmw<ABY, ASL>,ORA<ABY>,2>,//SLO
/* 0x1C */ NOP<ABX_pgx>,
/* 0x1D */ ORA<ABX_pgx>,
/* 0x1E */ rmw<ABX, ASL>,
/* 0x1F */ unofficial<rmw<ABX, ASL>,ORA<ABX>,2>,//SLO
/* 0x20 */ JSR,
/* 0x21 */ AND<IDX>,
/* 0x22 */ BAD_OP,
/* 0x23 */ unofficial<rmw<IDX, ROL>,AND<IDX>>,//RLA
/* 0x24 */ BIT<ZPG>,
/* 0x25 */ AND<ZPG>,
/* 0x26 */ rmw<ZPG, ROL>,
/* 0x27 */ unofficial<rmw<ZPG, ROL>,AND<ZPG>>,//RLA
/* 0x28 */ stack_pull<ProcStatus>,
/* 0x29 */ AND<IMM>,
/* 0x2A */ rmw<ACC, ROL>,
/* 0x2B */ unofficial<AND<IMM>,copyflag<N_FLAG,C_FLAG>,0>,//ANC
/* 0x2C */ BIT<ABS>,
/* 0x2D */ AND<ABS>,
/* 0x2E */ rmw<ABS, ROL>,
/* 0x2F */ unofficial<rmw<ABS, ROL>,AND<ABS>,2>,//RLA
/* 0x30 */ branch<if_set<N_FLAG>>,
/* 0x31 */ AND<IDY_pgx>,
/* 0x32 */ BAD_OP,
/* 0x33 */ unofficial<rmw<IDY, ROL>,AND<IDY>>,//RLA
/* 0x34 */ NOP<ZPX>,
/* 0x35 */ AND<ZPX>,
/* 0x36 */ rmw<ZPX, ROL>,
/* 0x37 */ unofficial<rmw<ZPX, ROL>,AND<ZPX>>,//RLA
/* 0x38 */ set<C_FLAG>,
/* 0x39 */ AND<ABY_pgx>,
/* 0x3A */ NOP,
/* 0x3B */ unofficial<rmw<ABY, ROL>,AND<ABY>,2>,//RLA
/* 0x3C */ NOP<ABX_pgx>,
/* 0x3D */ AND<ABX_pgx>,
/* 0x3E */ rmw<ABX, ROL>,
/* 0x3F */ unofficial<rmw<ABX, ROL>,AND<ABX>,2>,//RLA
/* 0x40 */ RTI,
/* 0x41 */ EOR<IDX>,
/* 0x42 */ BAD_OP,
/* 0x43 */ unofficial<rmw<IDX, LSR>,EOR<IDX>>,//SRE
/* 0x44 */ NOP<IMM>,
/* 0x45 */ EOR<ZPG>,
/* 0x46 */ rmw<ZPG, LSR>,
/* 0x47 */ unofficial<rmw<ZPG, LSR>,EOR<ZPG>>,//SRE
/* 0x48 */ stack_push<Accumulator>,
/* 0x49 */ EOR<IMM>,
/* 0x4A */ rmw<ACC, LSR>,
/* 0x4B */ unofficial<AND<IMM>,rmw<ACC, LSR>,0>, // ALR
/* 0x4C */ jump<ABS>,
/* 0x4D */ EOR<ABS>,
/* 0x4E */ rmw<ABS, LSR>,
/* 0x4F */ unofficial<rmw<ABS, LSR>,EOR<ABS>,2>,//SRE
/* 0x50 */ branch<if_clear<V_FLAG>>,
/* 0x51 */ EOR<IDY_pgx>,
/* 0x52 */ BAD_OP,
/* 0x53 */ unofficial<rmw<IDY, LSR>,EOR<IDY>>,//SRE
/* 0x54 */ NOP<ZPX>,
/* 0x55 */ EOR<ZPX>,
/* 0x56 */ rmw<ZPX, LSR>,
/* 0x57 */ unofficial<rmw<ZPX, LSR>,EOR<ZPX>>,//SRE
/* 0x58 */ clear<I_FLAG>,
/* 0x59 */ EOR<ABY_pgx>,
/* 0x5A */ NOP,
/* 0x5B */ unofficial<rmw<ABY, LSR>,EOR<ABY>,2>,//SRE
/* 0x5C */ NOP<ABX_pgx>,
/* 0x5D */ EOR<ABX_pgx>,
/* 0x5E */ rmw<ABX, LSR>,
/* 0x5F */ unofficial<rmw<ABX, LSR>,EOR<ABX>,2>,//SRE
/* 0x60 */ RTS,
/* 0x61 */ ADC<IDX>,
/* 0x62 */ BAD_OP,
/* 0x63 */ unofficial<rmw<IDX, ROR>,ADC<IDX>>,//RRA
/* 0x64 */ NOP<IMM>,
/* 0x65 */ ADC<ZPG>,
/* 0x66 */ rmw<ZPG, ROR>,
/* 0x67 */ unofficial<rmw<ZPG, ROR>,ADC<ZPG>>,//RRA
/* 0x68 */ stack_pull<Accumulator>,
/* 0x69 */ ADC<IMM>,
/* 0x6A */ rmw<ACC, ROR>,
/* 0x6B */ unofficial<AND<IMM>,rmw<ACC, ROR>,0>,//ARR
/* 0x6C */ jump<IND>,
/* 0x6D */ ADC<ABS>,
/* 0x6E */ rmw<ABS, ROR>,
/* 0x6F */ unofficial<rmw<ABS, ROR>,ADC<ABS>,2>,//RRA
/* 0x70 */ branch<if_set<V_FLAG>>,
/* 0x71 */ ADC<IDY_pgx>,
/* 0x72 */ BAD_OP,
/* 0x73 */ unofficial<rmw<IDY, ROR>,ADC<IDY>>,//RRA
/* 0x74 */ NOP<ZPX>,
/* 0x75 */ ADC<ZPX>,
/* 0x76 */ rmw<ZPX, ROR>,
/* 0x77 */ unofficial<rmw<ZPX, ROR>,ADC<ZPX>>,//RRA
/* 0x78 */ set<I_FLAG>,
/* 0x79 */ ADC<ABY_pgx>,
/* 0x7A */ NOP,
/* 0x7B */ unofficial<rmw<ABY, ROR>,ADC<ABY>,2>,//RRA
/* 0x7C */ NOP<ABX_pgx>,
/* 0x7D */ ADC<ABX_pgx>,
/* 0x7E */ rmw<ABX, ROR>,
/* 0x7F */ unofficial<rmw<ABX, ROR>,ADC<ABX>,2>,//RRA
/* 0x80 */ NOP<ZPG>,
/* 0x81 */ store<Accumulator,IDX>,
/* 0x82 */ NOP<IMM>,
/* 0x83 */ store<AX,IDX>,//SAX
/* 0x84 */ store<IndexRegY,ZPG>,
/* 0x85 */ store<Accumulator,ZPG>,
/* 0x86 */ store<IndexRegX,ZPG>,
/* 0x87 */ store<AX,ZPG>,//SAX
/* 0x88 */ rmw<Y__, DEC>,	
/* 0x89 */ NOP<IMM>,
/* 0x8A */ transfer<IndexRegX,Accumulator>,
/* 0x8B */ NOP<IMM>,//XAA
/* 0x8C */ store<IndexRegY,ABS>,
/* 0x8D */ store<Accumulator,ABS>,
/* 0x8E */ store<IndexRegX,ABS>,
/* 0x8F */ store<AX,ABS>,//SAX
/* 0x90 */ branch<if_clear<C_FLAG>>,
/* 0x91 */ store<Accumulator,IDY>,
/* 0x92 */ BAD_OP,
/* 0x93 */ NOP<IDY>,//AHX<IDY>
/* 0x94 */ store<IndexRegY,ZPX>,
/* 0x95 */ store<Accumulator,ZPX>,
/* 0x96 */ store<IndexRegX,ZPY>,
/* 0x97 */ store<AX,ZPY>,//SAX
/* 0x98 */ transfer<IndexRegY,Accumulator>,
/* 0x99 */ store<Accumulator,ABY>,
/* 0x9A */ transfer<IndexRegX,StackPointer>,
/* 0x9B */ NOP<ABY>,//TAS<ABY>
/* 0x9C */ NOP<ABX>,//SHY<ABX>
/* 0x9D */ store<Accumulator,ABX>,
/* 0x9E */ NOP<ABY>,//SHX<ABY>
/* 0x9F */ NOP<ABY>,//AHX<ABY>
/* 0xA0 */ load<IndexRegY,IMM>,
/* 0xA1 */ load<Accumulator,IDX>,
/* 0xA2 */ load<IndexRegX,IMM>,
/* 0xA3 */ unofficial<load<Accumulator,IDX>,transfer<Accumulator,IndexRegX>,0>,//LAX
/* 0xA4 */ load<IndexRegY,ZPG>,
/* 0xA5 */ load<Accumulator,ZPG>,
/* 0xA6 */ load<IndexRegX,ZPG>,
/* 0xA7 */ unofficial<load<Accumulator,ZPG>,transfer<Accumulator,IndexRegX>,0>,//LAX
/* 0xA8 */ transfer<Accumulator,IndexRegY>,
/* 0xA9 */ load<Accumulator,IMM>,
/* 0xAA */ transfer<Accumulator,IndexRegX>,
/* 0xAB */ unofficial<load<Accumulator,IMM>,transfer<Accumulator,IndexRegX>,0>,//LAX
/* 0xAC */ load<IndexRegY,ABS>,
/* 0xAD */ load<Accumulator,ABS>,
/* 0xAE */ load<IndexRegX,ABS>,
/* 0xAF */ unofficial<load<Accumulator,ABS>,transfer<Accumulator,IndexRegX>,0>,//LAX
/* 0xB0 */ branch<if_set<C_FLAG>>,
/* 0xB1 */ load<Accumulator,IDY_pgx>,
/* 0xB2 */ BAD_OP,
/* 0xB3 */ unofficial<load<Accumulator,IDY_pgx>,transfer<Accumulator,IndexRegX>,0>,//LAX
/* 0xB4 */ load<IndexRegY,ZPX>,
/* 0xB5 */ load<Accumulator,ZPX>,
/* 0xB6 */ load<IndexRegX,ZPY>,
/* 0xB7 */ unofficial<load<Accumulator,ZPY>,transfer<Accumulator,IndexRegX>,0>,//LAX
/* 0xB8 */ clear<V_FLAG>,
/* 0xB9 */ load<Accumulator,ABY_pgx>,
/* 0xBA */ transfer<StackPointer,IndexRegX>,
/* 0xBB */ NOP<ABY>, //LAS a,y
/* 0xBC */ load<IndexRegY,ABX_pgx>,
/* 0xBD */ load<Accumulator,ABX_pgx>,
/* 0xBE */ load<IndexRegX,ABY_pgx>,
/* 0xBF */ unofficial<load<Accumulator,ABY_pgx>,transfer<Accumulator,IndexRegX>,0>,//LAX
/* 0xC0 */ compare<IndexRegY,IMM>,
/* 0xC1 */ compare<Accumulator,IDX>,
/* 0xC2 */ NOP<IMM>,
/* 0xC3 */ unofficial<rmw<IDX, DEC>,compare<Accumulator,IDX>>,//DCP
/* 0xC4 */ compare<IndexRegY,ZPG>,
/* 0xC5 */ compare<Accumulator,ZPG>,
/* 0xC6 */ rmw<ZPG, DEC>,
/* 0xC7 */ unofficial<rmw<ZPG, DEC>,compare<Accumulator,ZPG>>,//DCP
/* 0xC8 */ rmw<Y__, INC>,
/* 0xC9 */ compare<Accumulator,IMM>,
/* 0xCA */ rmw<X__, DEC>,
/* 0xCB */ transfer<AX,Accumulator>,//AXS
/* 0xCC */ compare<IndexRegY,ABS>,
/* 0xCD */ compare<Accumulator,ABS>,
/* 0xCE */ rmw<ABS, DEC>,
/* 0xCF */ unofficial<rmw<ABS, DEC>,compare<Accumulator,ABS>,2>,//DCP
/* 0xD0 */ branch<if_clear<Z_FLAG>>,
/* 0xD1 */ compare<Accumulator,IDY_pgx>,
/* 0xD2 */ BAD_OP,
/* 0xD3 */ unofficial<rmw<IDY, DEC>,compare<Accumulator,IDY>>,//DCP
/* 0xD4 */ NOP<ZPX>,
/* 0xD5 */ compare<Accumulator,ZPX>,
/* 0xD6 */ rmw<ZPX, DEC>,
/* 0xD7 */ unofficial<rmw<ZPX, DEC>,compare<Accumulator,ZPX>>,//DCP
/* 0xD8 */ clear<D_FLAG>,
/* 0xD9 */ compare<Accumulator,ABY_pgx>,
/* 0xDA */ NOP,
/* 0xDB */ unofficial<rmw<ABY, DEC>,compare<Accumulator,ABY>,2>,//DCP
/* 0xDC */ NOP<ABX_pgx>,
/* 0xDD */ compare<Accumulator,ABX_pgx>,
/* 0xDE */ rmw<ABX, DEC>,
/* 0xDF */ unofficial<rmw<ABX, DEC>,compare<Accumulator,ABX>,2>,//DCP
/* 0xE0 */ compare<IndexRegX,IMM>,
/* 0xE1 */ SBC<IDX>,
/* 0xE2 */ NOP<IMM>,
/* 0xE3 */ unofficial<rmw<IDX, INC>,SBC<IDX>>,//ISB(ISC)
/* 0xE4 */ compare<IndexRegX,ZPG>,
/* 0xE5 */ SBC<ZPG>,
/* 0xE6 */ rmw<ZPG, INC>,
/* 0xE7 */ unofficial<rmw<ZPG, INC>,SBC<ZPG>>,//ISB(ISC)
/* 0xE8 */ rmw<X__, INC>,
/* 0xE9 */ SBC<IMM>,
/* 0xEA */ NOP,
/* 0xEB */ SBC<IMM>,
/* 0xEC */ compare<IndexRegX,ABS>,
/* 0xED */ SBC<ABS>,
/* 0xEE */ rmw<ABS, INC>,
/* 0xEF */ unofficial<rmw<ABS, INC>,SBC<ABS>,2>,//ISB(ISC)
/* 0xF0 */ branch<if_set<Z_FLAG>>,
/* 0xF1 */ SBC<IDY_pgx>,
/* 0xF2 */ BAD_OP,
/* 0xF3 */ unofficial<rmw<IDY, INC>,SBC<IDY>>,//ISB(ISC)
/* 0xF4 */ NOP<ZPX>,
/* 0xF5 */ SBC<ZPX>,
/* 0xF6 */ rmw<ZPX, INC>,
/* 0xF7 */ unofficial<rmw<ZPX, INC>,SBC<ZPX>>,//ISB(ISC)
/* 0xF8 */ set<D_FLAG>,
/* 0xF9 */ SBC<ABY_pgx>,
/* 0xFA */ NOP,
/* 0xFB */ unofficial<rmw<ABY, INC>,SBC<ABY>,2>,//ISB(ISC)
/* 0xFC */ NOP<ABX_pgx>,
/* 0xFD */ SBC<ABX_pgx>,
/* 0xFE */ rmw<ABX, INC>,
/* 0xFF */ unofficial<rmw<ABX, INC>,SBC<ABX>,2>//ISB(ISC)
};

#undef IDX
#undef ACC
#undef X__
#undef Y__
#undef IMM
#undef ZPG
#undef ZPX
#undef ZPY
#undef ABS
#undef ABX
#undef ABY
#undef IDX
#undef IDY
#undef IND
#undef BAD_OP
#undef JSR
#undef BRK
#undef RTS
#undef RTI
#undef NOP
#undef Accumulator
#undef IndexRegX
#undef IndexRegY
#undef StackPointer
#undef ProcStatus
#undef Accumulator
#undef IndexRegX
#undef IndexRegY
#undef StackPointer
#undef ProcStatus
#undef rmw
#undef AX
#undef ABY_pgx
#undef ABX_pgx
#undef IDY_pgx
#undef load
#undef store
#undef transfer
#undef stack_push
#undef stack_pull
#undef set
#undef clear
#undef if_clear
#undef if_set
#undef AND
#undef EOR
#undef ORA
#undef BIT
#undef compare
#undef ADC
#undef SBC
#undef INC
#undef DEC
#undef ASL
#undef LSR
#undef ROL
#undef ROR
#undef jump
#undef branch
#undef copyflag
#undef unofficial
#undef N_FLAG
#undef V_FLAG
#undef D_FLAG
#undef I_FLAG
#undef Z_FLAG
#undef C_FLAG 
