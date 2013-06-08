// ops template definitions ...within CPU class


// load/store/transfer:

template<regw R, mode M> inline void load(){
  (this->*R)(read<M>());
}

template<regr R, mode M> inline void store(){
  write((this->*R)(),(this->*M)());
}

template<regr from, regw to> inline void transfer(){
  (this->*to)((this->*from)());
}

// stack
template<regr from> inline void stack_push(){
  push((this->*from)());
}

template<regw to> inline void stack_pull(){
  (this->*to)(pull());
}

// status:

template<CPU::Flag F> inline void set(){
  P|=F;
}

template<CPU::Flag F> inline void clear(){
  P&=~F;
}

// conditions:

template<CPU::Flag F> inline uint8_t if_clear(){
  return !(P&F);
}
  
template<CPU::Flag F> inline uint8_t if_set(){
  return P&F;
}

// logical:

template<mode M> inline void AND(){
  setZN(A&=read<M>());
}

template<mode M> inline void EOR(){
  setZN(A^=read<M>());
}

template<mode M> inline void ORA(){
  setZN(A|=read<M>());
}

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
  int r = A + m + (P&C_FLAG);
  set_if<V_FLAG>((((A^m)&0x80)^0x80)&((A^r)&0x80));
  set_if<C_FLAG>(r >> 8);
  setZN(A = r);
}

template<mode M> inline void SBC(){
  uint8_t m = read<M>();
  int r=A - m - ((P&C_FLAG)^1);
  set_if<V_FLAG>((A^r)&(A^m)&0x80);
  set_if<C_FLAG>(((r>>8)&C_FLAG)^C_FLAG);
  setZN(A = r);
}

// increment/decrement:

template<mode M> inline void INC(){
  setZN(++getref<M>());
}

template<mode M> inline void DEC(){
  setZN(--getref<M>());
}


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

template<mode M> inline void jump(){
  PC = (this->*M)();
}

template<condition C> inline void branch(){
  if((this->*C)()){
    PC = sum_check_pgx<char>(PC, next());
    addcyc();
  } else PC++;
  
}


// no-op:

template<mode M> inline void NOP(){
  (this->*M)();
}


// unofficial - dual function:

template<CPU::Flag from, CPU::Flag to> inline void copyflag(){
  set_if<to>(P&from);
}

template<op F1, op F2> inline void unofficial(){
  (this->*F1)();
  (this->*F2)();
}

template<op F1, op F2> inline void unofficial_rmw(){
  (this->*F1)();
  PC--;
  (this->*F2)();
}

template<op F1, op F2, int I> inline void unofficial_rmw(){ // bleh
  (this->*F1)();
  PC-=I;
  (this->*F2)();
}
