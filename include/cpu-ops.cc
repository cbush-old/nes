// load / store / transfer:
//
//
template<regw R, mode M> inline void load() {
  (this->*R)(read<M>());
}

template<regr R, mode M> inline void store() {
  write((this->*R)(),(this->*M)());
}

template<regr from, regw to> inline void transfer() {
  (this->*to)((this->*from)());
}


// stack:
//
//
template<regr from> inline void stack_push() {
  push((this->*from)());
}

template<regw to> inline void stack_pull() {
  (this->*to)(pull());
}


// status:
//
//
template<CPU::Flag F> inline void set() {
  P |= F;
}

template<CPU::Flag F> inline void clear() {
  P &= ~F;
}


// conditions:
//
//
template<CPU::Flag F> inline uint8_t if_clear() {
  return !(P & F);
}

template<CPU::Flag F> inline uint8_t if_set() {
  return P & F;
}


// logical:
//
//
template<mode M> inline void AND() {
  setZN(A &= read<M>());
}

template<mode M> inline void EOR() {
  setZN(A ^= read<M>());
}

template<mode M> inline void ORA() {
  setZN(A |= read<M>());
}

template<mode M> inline void BIT() {
  uint8_t m = read<M>();
  set_if<N_FLAG>(m&0x80);
  set_if<V_FLAG>(m&0x40);
  set_if<Z_FLAG>(!(A&m));
}

template<regr R, mode M> inline void compare() {
  int m = (this->*R)() - read<M>();
  set_if<C_FLAG>(m >= 0);
  setZN((uint8_t)m);
}


// arithmetic:
//
//
template<mode M> inline void ADC() {
  uint8_t m = read<M>();
  int r = A + m + !!(P&C_FLAG);
  set_if<V_FLAG>(~(A^m) & (A^r) & 0x80);
  set_if<C_FLAG>(r >> 8);
  setZN(A = r);
}

template<mode M> inline void SBC() {
  uint8_t m = read<M>();
  int r = A - m - ((P&C_FLAG)^1);
  set_if<V_FLAG>((A^m) & (A^r) & 0x80);
  set_if<C_FLAG>(!((r>>8)&C_FLAG));
  setZN(A = r);
}

template<mode M, rmw_op f>
inline void rmw() {
  if (M == &CPU::ACC) {

    setZN(A = (this->*f)(A));

  } else if (M == &CPU::X__) {

    setZN(X = (this->*f)(X));

  } else if (M == &CPU::Y__) {

    setZN(Y = (this->*f)(Y));

  } else {

    uint16_t addr = (this->*M)();
    uint8_t value = (this->*f)(read(addr));
    setZN(value);
    write(value, addr);

  }
}

// increment / decrement:
//
//
uint8_t INC(uint8_t v) { // rmw
  return v + 1;
}

uint8_t DEC(uint8_t v) { // rmw
  return v - 1;
}


// bit shift:
//
//
uint8_t ASL(uint8_t v) { // rmw
  set_if<C_FLAG>(v & 0x80);
  return v << 1;
}

uint8_t LSR(uint8_t v) { // rmw
  set_if<C_FLAG>(v & 1);
  return v >> 1;
}

uint8_t ROL(uint8_t v) { // rmw
  uint8_t tmp = P & C_FLAG;
  set_if<C_FLAG>(v & 0x80);
  return (v << 1) | tmp;
}

uint8_t ROR(uint8_t v) { // rmw
  uint8_t tmp = P&C_FLAG;
  set_if<C_FLAG>(v & 1);
  return (v >> 1) | (tmp << 7);
}


// jump/branch:
//
//
template<mode M> inline void jump() {
  PC = (this->*M)();
}

template<condition C> inline void branch() {

  if((this->*C)()) {
    PC = sum_check_pgx<char>(PC, next());
    addcyc();
  } else {
    ++PC;
  }

}


// no-op: still advances the program counter
//
//
template<mode M> inline void NOP() {
  (this->*M)();
}


// unofficial - dual function:
//
//
template<CPU::Flag from, CPU::Flag to> inline void copyflag() {
  set_if<to>(P&from);
}

template<op F1, op F2, int I=1> inline void unofficial() {
  (this->*F1)();
  PC -= I; // bleh
  (this->*F2)();
}


// non-templated:
//
//
inline void JSR() {
  push2(PC + 1);
  PC = ABS();
}

inline void RTS() {
  PC = pull2() + 1;
}

inline void BRK() {
  push2(PC + 1);
  stack_push<&CPU::ProcStatus>();
  PC = read(0xfffe) | (read(0xffff)<<8);
}

inline void RTI() {
  stack_pull<&CPU::ProcStatus>();
  PC = pull2();
}

inline void NOP() {
  /* do nothing */
}

inline void BAD_OP() {
  print_status();
}
