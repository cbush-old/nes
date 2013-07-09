// load/store/transfer:
template<regw R, mode M> inline void load(){ tick(); (this->*R)(read<M>()); }
template<regr R, mode M> inline void store(){ tick(); write((this->*R)(),(this->*M)()); }
template<regr from, regw to> inline void transfer(){ (this->*to)((this->*from)()); }

// stack
template<regr from> inline void stack_push(){ push((this->*from)()); }
template<regw to> inline void stack_pull(){ (this->*to)(pull()); }

// status:
template<CPU::Flag F> inline void set(){ tick(); tick(); P|=F; }
template<CPU::Flag F> inline void clear(){ tick(); tick(); P&=~F; }

// conditions:
template<CPU::Flag F> inline uint8_t if_clear(){ return !(P&F); }
template<CPU::Flag F> inline uint8_t if_set(){ return P&F; }

// logical:
template<mode M> inline void AND(){ setZN(A&=read<M>()); }
template<mode M> inline void EOR(){ setZN(A^=read<M>()); }
template<mode M> inline void ORA(){ setZN(A|=read<M>()); }
template<mode M> inline void BIT(){
  uint8_t m = read<M>();
  set_if<N_FLAG>(m&0x80);
  set_if<V_FLAG>(m&0x40);
  set_if<Z_FLAG>(!(A&m));
}
template<regr R, mode M> inline void compare(){
  int m = (this->*R)() - read<M>();
  set_if<C_FLAG>(m >= 0);
  setZN((uint8_t)m);
}

// arithmetic:
template<mode M> inline void ADC(){
  uint8_t m = read<M>();
  int r = A + m + !!(P&C_FLAG);
  set_if<V_FLAG>(~(A^m) & (A^r) & 0x80);
  set_if<C_FLAG>(r >> 8);
  setZN(A = r);
}
template<mode M> inline void SBC(){
  uint8_t m = read<M>();
  int r = A - m - ((P&C_FLAG)^1);
  set_if<V_FLAG>((A^m) & (A^r) & 0x80);
  set_if<C_FLAG>(!((r>>8)&C_FLAG));
  setZN(A = r);
}

// increment/decrement:
template<mode M> inline void INC(){ setZN(++getref<M>()); }
template<mode M> inline void DEC(){ setZN(--getref<M>()); }

// bit shift:
template<mode M> inline void ASL(){
  uint8_t& target = getref<M>();
  set_if<C_FLAG>(target & 0x80);
  setZN(target <<= 1);
}
template<mode M> inline void LSR(){
  uint8_t& target = getref<M>();
  set_if<C_FLAG>(target&0x01);
  setZN(target >>= 1);
}
template<mode M> inline void ROL(){
  uint8_t& target = getref<M>();
  uint8_t tmp = P&C_FLAG;
  set_if<C_FLAG>(target&0x80);
  setZN((target<<=1)|=tmp);
}
template<mode M> inline void ROR(){
  uint8_t& target = getref<M>();
  uint8_t tmp = P&C_FLAG;
  set_if<C_FLAG>(target&0x01);
  setZN((target>>=1)|=tmp<<7);
}

// jump/branch:
template<mode M> inline void jump(){ PC = (this->*M)(); }
template<condition C> inline void branch(){
  tick(); 
  if((this->*C)()){
    PC = sum_check_pgx<char>(PC, next());
    tick(); 
    addcyc();
  } else PC++;
}

// no-op: still advances the program counter
template<mode M> inline void NOP(){ (this->*M)(); }

// unofficial - dual function:
template<CPU::Flag from, CPU::Flag to> inline void copyflag(){ set_if<to>(P&from); }
template<op F1, op F2, int I=1> inline void unofficial(){ // bleh
  (this->*F1)();
  PC-=I;
  (this->*F2)();
}

// non-templated
inline void JSR(){
  push2(PC + 1);
  PC = ABS();
}
inline void RTS(){
  PC = pull2() + 1;
}
inline void BRK(){
  push2(PC + 1);
  stack_push<&CPU::ProcStatus>();
  PC = read(0xfffe) | (read(0xffff)<<8);
}
inline void RTI(){
  stack_pull<&CPU::ProcStatus>();
  PC = pull2();
}
inline void NOP(){}
inline void BAD_OP(){
  throw std::runtime_error("Kil");
}
