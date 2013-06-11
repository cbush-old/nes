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
/* 0x03 */ unofficial<ASL<IDX>,ORA<IDX>>,//SLO
/* 0x04 */ NOP<IMM>,
/* 0x05 */ ORA<ZPG>,
/* 0x06 */ ASL<ZPG>,
/* 0x07 */ unofficial<ASL<ZPG>,ORA<ZPG>>,//SLO
/* 0x08 */ stack_push<ProcStatus>,	
/* 0x09 */ ORA<IMM>,
/* 0x0A */ ASL<ACC>,
/* 0x0B */ unofficial<AND<IMM>,copyflag<N_FLAG,C_FLAG>,0>,//ANC
/* 0x0C */ NOP<ABS>,
/* 0x0D */ ORA<ABS>,
/* 0x0E */ ASL<ABS>,
/* 0x0F */ unofficial<ASL<ABS>,ORA<ABS>,2>,//SLO
/* 0x10 */ branch<if_clear<N_FLAG>>,
/* 0x11 */ ORA<IDY_pgx>,
/* 0x12 */ BAD_OP,
/* 0x13 */ unofficial<ASL<IDY>,ORA<IDY>>,//SLO
/* 0x14 */ NOP<ZPX>,
/* 0x15 */ ORA<ZPX>,
/* 0x16 */ ASL<ZPX>,
/* 0x17 */ unofficial<ASL<ZPX>,ORA<ZPX>>,//SLO
/* 0x18 */ clear<C_FLAG>,
/* 0x19 */ ORA<ABY_pgx>,
/* 0x1A */ NOP,
/* 0x1B */ unofficial<ASL<ABY>,ORA<ABY>,2>,//SLO
/* 0x1C */ NOP<ABX_pgx>,
/* 0x1D */ ORA<ABX_pgx>,
/* 0x1E */ ASL<ABX>,
/* 0x1F */ unofficial<ASL<ABX>,ORA<ABX>,2>,//SLO
/* 0x20 */ JSR,
/* 0x21 */ AND<IDX>,
/* 0x22 */ BAD_OP,
/* 0x23 */ unofficial<ROL<IDX>,AND<IDX>>,//RLA
/* 0x24 */ BIT<ZPG>,
/* 0x25 */ AND<ZPG>,
/* 0x26 */ ROL<ZPG>,
/* 0x27 */ unofficial<ROL<ZPG>,AND<ZPG>>,//RLA
/* 0x28 */ stack_pull<ProcStatus>,
/* 0x29 */ AND<IMM>,
/* 0x2A */ ROL<ACC>,
/* 0x2B */ unofficial<AND<IMM>,copyflag<N_FLAG,C_FLAG>,0>,//ANC
/* 0x2C */ BIT<ABS>,
/* 0x2D */ AND<ABS>,
/* 0x2E */ ROL<ABS>,
/* 0x2F */ unofficial<ROL<ABS>,AND<ABS>,2>,//RLA
/* 0x30 */ branch<if_set<N_FLAG>>,
/* 0x31 */ AND<IDY_pgx>,
/* 0x32 */ BAD_OP,
/* 0x33 */ unofficial<ROL<IDY>,AND<IDY>>,//RLA
/* 0x34 */ NOP<ZPX>,
/* 0x35 */ AND<ZPX>,
/* 0x36 */ ROL<ZPX>,
/* 0x37 */ unofficial<ROL<ZPX>,AND<ZPX>>,//RLA
/* 0x38 */ set<C_FLAG>,
/* 0x39 */ AND<ABY_pgx>,
/* 0x3A */ NOP,
/* 0x3B */ unofficial<ROL<ABY>,AND<ABY>,2>,//RLA
/* 0x3C */ NOP<ABX_pgx>,
/* 0x3D */ AND<ABX_pgx>,
/* 0x3E */ ROL<ABX>,
/* 0x3F */ unofficial<ROL<ABX>,AND<ABX>,2>,//RLA
/* 0x40 */ RTI,
/* 0x41 */ EOR<IDX>,
/* 0x42 */ BAD_OP,
/* 0x43 */ unofficial<LSR<IDX>,EOR<IDX>>,//SRE
/* 0x44 */ NOP<IMM>,
/* 0x45 */ EOR<ZPG>,
/* 0x46 */ LSR<ZPG>,
/* 0x47 */ unofficial<LSR<ZPG>,EOR<ZPG>>,//SRE
/* 0x48 */ stack_push<Accumulator>,
/* 0x49 */ EOR<IMM>,
/* 0x4A */ LSR<ACC>,
/* 0x4B */ unofficial<AND<IMM>,LSR<ACC>,0>, // ALR
/* 0x4C */ jump<ABS>,
/* 0x4D */ EOR<ABS>,
/* 0x4E */ LSR<ABS>,
/* 0x4F */ unofficial<LSR<ABS>,EOR<ABS>,2>,//SRE
/* 0x50 */ branch<if_clear<V_FLAG>>,
/* 0x51 */ EOR<IDY_pgx>,
/* 0x52 */ BAD_OP,
/* 0x53 */ unofficial<LSR<IDY>,EOR<IDY>>,//SRE
/* 0x54 */ NOP<ZPX>,
/* 0x55 */ EOR<ZPX>,
/* 0x56 */ LSR<ZPX>,
/* 0x57 */ unofficial<LSR<ZPX>,EOR<ZPX>>,//SRE
/* 0x58 */ clear<I_FLAG>,
/* 0x59 */ EOR<ABY_pgx>,
/* 0x5A */ NOP,
/* 0x5B */ unofficial<LSR<ABY>,EOR<ABY>,2>,//SRE
/* 0x5C */ NOP<ABX_pgx>,
/* 0x5D */ EOR<ABX_pgx>,
/* 0x5E */ LSR<ABX>,
/* 0x5F */ unofficial<LSR<ABX>,EOR<ABX>,2>,//SRE
/* 0x60 */ RTS,
/* 0x61 */ ADC<IDX>,
/* 0x62 */ BAD_OP,
/* 0x63 */ unofficial<ROR<IDX>,ADC<IDX>>,//RRA
/* 0x64 */ NOP<IMM>,
/* 0x65 */ ADC<ZPG>,
/* 0x66 */ ROR<ZPG>,
/* 0x67 */ unofficial<ROR<ZPG>,ADC<ZPG>>,//RRA
/* 0x68 */ stack_pull<Accumulator>,
/* 0x69 */ ADC<IMM>,
/* 0x6A */ ROR<ACC>,
/* 0x6B */ unofficial<AND<IMM>,ROR<ACC>,0>,//ARR
/* 0x6C */ jump<IND>,
/* 0x6D */ ADC<ABS>,
/* 0x6E */ ROR<ABS>,
/* 0x6F */ unofficial<ROR<ABS>,ADC<ABS>,2>,//RRA
/* 0x70 */ branch<if_set<V_FLAG>>,
/* 0x71 */ ADC<IDY_pgx>,
/* 0x72 */ BAD_OP,
/* 0x73 */ unofficial<ROR<IDY>,ADC<IDY>>,//RRA
/* 0x74 */ NOP<ZPX>,
/* 0x75 */ ADC<ZPX>,
/* 0x76 */ ROR<ZPX>,
/* 0x77 */ unofficial<ROR<ZPX>,ADC<ZPX>>,//RRA
/* 0x78 */ set<I_FLAG>,
/* 0x79 */ ADC<ABY_pgx>,
/* 0x7A */ NOP,
/* 0x7B */ unofficial<ROR<ABY>,ADC<ABY>,2>,//RRA
/* 0x7C */ NOP<ABX_pgx>,
/* 0x7D */ ADC<ABX_pgx>,
/* 0x7E */ ROR<ABX>,
/* 0x7F */ unofficial<ROR<ABX>,ADC<ABX>,2>,//RRA
/* 0x80 */ NOP<ZPG>,
/* 0x81 */ store<Accumulator,IDX>,
/* 0x82 */ NOP<IMM>,
/* 0x83 */ store<AX,IDX>,//SAX
/* 0x84 */ store<IndexRegY,ZPG>,
/* 0x85 */ store<Accumulator,ZPG>,
/* 0x86 */ store<IndexRegX,ZPG>,
/* 0x87 */ store<AX,ZPG>,//SAX
/* 0x88 */ DEC<Y__>,	
/* 0x89 */ BAD_OP,
/* 0x8A */ transfer<IndexRegX,Accumulator>,
/* 0x8B */ BAD_OP,
/* 0x8C */ store<IndexRegY,ABS>,
/* 0x8D */ store<Accumulator,ABS>,
/* 0x8E */ store<IndexRegX,ABS>,
/* 0x8F */ store<AX,ABS>,//SAX
/* 0x90 */ branch<if_clear<C_FLAG>>,
/* 0x91 */ store<Accumulator,IDY>,
/* 0x92 */ BAD_OP,
/* 0x93 */ BAD_OP,//AHX<IDY>
/* 0x94 */ store<IndexRegY,ZPX>,
/* 0x95 */ store<Accumulator,ZPX>,
/* 0x96 */ store<IndexRegX,ZPY>,
/* 0x97 */ store<AX,ZPY>,//SAX
/* 0x98 */ transfer<IndexRegY,Accumulator>,
/* 0x99 */ store<Accumulator,ABY>,
/* 0x9A */ transfer<IndexRegX,StackPointer>,
/* 0x9B */ BAD_OP,//TAS<ABY>
/* 0x9C */ BAD_OP,//SHY<ABX>
/* 0x9D */ store<Accumulator,ABX>,
/* 0x9E */ BAD_OP,//SHX<ABY>
/* 0x9F */ BAD_OP,//AHX<ABY>
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
/* 0xBB */ BAD_OP,
/* 0xBC */ load<IndexRegY,ABX_pgx>,
/* 0xBD */ load<Accumulator,ABX_pgx>,
/* 0xBE */ load<IndexRegX,ABY_pgx>,
/* 0xBF */ unofficial<load<Accumulator,ABY_pgx>,transfer<Accumulator,IndexRegX>,0>,//LAX
/* 0xC0 */ compare<IndexRegY,IMM>,
/* 0xC1 */ compare<Accumulator,IDX>,
/* 0xC2 */ BAD_OP,
/* 0xC3 */ unofficial<DEC<IDX>,compare<Accumulator,IDX>>,//DCP
/* 0xC4 */ compare<IndexRegY,ZPG>,
/* 0xC5 */ compare<Accumulator,ZPG>,
/* 0xC6 */ DEC<ZPG>,
/* 0xC7 */ unofficial<DEC<ZPG>,compare<Accumulator,ZPG>>,//DCP
/* 0xC8 */ INC<Y__>,
/* 0xC9 */ compare<Accumulator,IMM>,
/* 0xCA */ DEC<X__>,
/* 0xCB */ transfer<AX,Accumulator>,//AXS
/* 0xCC */ compare<IndexRegY,ABS>,
/* 0xCD */ compare<Accumulator,ABS>,
/* 0xCE */ DEC<ABS>,
/* 0xCF */ unofficial<DEC<ABS>,compare<Accumulator,ABS>,2>,//DCP
/* 0xD0 */ branch<if_clear<Z_FLAG>>,
/* 0xD1 */ compare<Accumulator,IDY_pgx>,
/* 0xD2 */ BAD_OP,
/* 0xD3 */ unofficial<DEC<IDY>,compare<Accumulator,IDY>>,//DCP
/* 0xD4 */ NOP<ZPX>,
/* 0xD5 */ compare<Accumulator,ZPX>,
/* 0xD6 */ DEC<ZPX>,
/* 0xD7 */ unofficial<DEC<ZPX>,compare<Accumulator,ZPX>>,//DCP
/* 0xD8 */ clear<D_FLAG>,
/* 0xD9 */ compare<Accumulator,ABY_pgx>,
/* 0xDA */ NOP,
/* 0xDB */ unofficial<DEC<ABY>,compare<Accumulator,ABY>,2>,//DCP
/* 0xDC */ NOP<ABX_pgx>,
/* 0xDD */ compare<Accumulator,ABX_pgx>,
/* 0xDE */ DEC<ABX>,
/* 0xDF */ unofficial<DEC<ABX>,compare<Accumulator,ABX>,2>,//DCP
/* 0xE0 */ compare<IndexRegX,IMM>,
/* 0xE1 */ SBC<IDX>,
/* 0xE2 */ BAD_OP,
/* 0xE3 */ unofficial<INC<IDX>,SBC<IDX>>,//ISB(ISC)
/* 0xE4 */ compare<IndexRegX,ZPG>,
/* 0xE5 */ SBC<ZPG>,
/* 0xE6 */ INC<ZPG>,
/* 0xE7 */ unofficial<INC<ZPG>,SBC<ZPG>>,//ISB(ISC)
/* 0xE8 */ INC<X__>,
/* 0xE9 */ SBC<IMM>,
/* 0xEA */ NOP,
/* 0xEB */ SBC<IMM>,
/* 0xEC */ compare<IndexRegX,ABS>,
/* 0xED */ SBC<ABS>,
/* 0xEE */ INC<ABS>,
/* 0xEF */ unofficial<INC<ABS>,SBC<ABS>,2>,//ISB(ISC)
/* 0xF0 */ branch<if_set<Z_FLAG>>,
/* 0xF1 */ SBC<IDY_pgx>,
/* 0xF2 */ BAD_OP,
/* 0xF3 */ unofficial<INC<IDY>,SBC<IDY>>,//ISB(ISC)
/* 0xF4 */ NOP<ZPX>,
/* 0xF5 */ SBC<ZPX>,
/* 0xF6 */ INC<ZPX>,
/* 0xF7 */ unofficial<INC<ZPX>,SBC<ZPX>>,//ISB(ISC)
/* 0xF8 */ set<D_FLAG>,
/* 0xF9 */ SBC<ABY_pgx>,
/* 0xFA */ NOP,
/* 0xFB */ unofficial<INC<ABY>,SBC<ABY>,2>,//ISB(ISC)
/* 0xFC */ NOP<ABX_pgx>,
/* 0xFD */ SBC<ABX_pgx>,
/* 0xFE */ INC<ABX>,
/* 0xFF */ unofficial<INC<ABX>,SBC<ABX>,2>//ISB(ISC)
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
