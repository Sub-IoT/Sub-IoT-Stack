/////////////dsp.h/////////////////
/////////////     /////////////////
//with the "simple" instruction which are described below  we have some parallel move instruction which can be loaded in parallel whith the "simple" instruction
// those instruction are detemined in the operand value (cp_opa,cp_opb).

//they are:

//X:     Description : Move register to Xmemory  or Xmemory to register  and update address for addresses  génération  ALU X. 

//Y:     Description : Move register to Ymemory  or Ymemory to register  and update address for addresses  génération  ALU Y.
 
//X_Y:   Description : Move register to Xmemory  or Xmemory to register  and update address for addresses  génération  ALU X;
//Move register to Ymemory  or Ymemory to register  and update address for addresses  génération  ALU Y. 

//U:     Description : Update the address register according to the effective addressing mode. All update addressing modes may be used.

// for more instructions details (opcode,operand value,parallel move) look copro_dsp_doc
/**************************************************************************************************************************/




//add(cp_opa,cp_opb)    // Description: Add the source operand S to the destination operand Dst and store the result in the destination register Dst.

static inline void add(int a, int b)
{
    __asm__ __volatile__ ("cp1\tr0, %0, %1 , #6"
	:
	:"r" (a), "r" (b));
}

//sub(cp_opa,cp_opb)               //Description: Subtract the source operand S from the destination operand Dst and storethe result in the destination operand Dst.
static inline void sub(int a, int b)
{
    __asm__ __volatile__ ("cp1\tr0, %0, %1 , #9"
	:
	:"r" (a), "r" (b));
}
//shift_l(cp_opa,cp_opb)           //Description:  Shift the destination register Dst  one bit to left 
static inline void shift_l(int a, int b)
{
    __asm__ __volatile__ ("cp1\tr0, %0, %1 , #10"
	:
	:"r" (a), "r" (b));
}
//shift_r(cp_opa,cp_opb)           //Description:  Shift the destination register Dst  one bit to right 
static inline void shift_r(int a, int b)
{
    __asm__ __volatile__ ("cp1\tr0, %0, %1 , #11"
	:
	:"r" (a), "r" (b));
}
//lua(cp_opa,cp_opb)               //Description :Load the updated address into the destination address register Dst.
static inline void lua(int a, int b)
{
    __asm__ __volatile__ ("cp1\tr0, %0, %1 , #8"
	:
	:"r" (a), "r" (b));
}
//mult(cp_opa,cp_opb)              //Description : Multiply the  source operands S1 and S2 and store the product to the destination register Dst.
static inline void mult(int a, int b)
{
    __asm__ __volatile__ ("cp1\tr0, %0, %1 , #5"
	:
	:"r" (a), "r" (b));
}
//mac(cp_opa,cp_opb)               //Description :Multiply the  source operands S1 and S2 and add the product to the  destination register Dst.


static inline void mac(int a, int b)
{
    
    __asm__ __volatile__ ("cp1\tr0, %0, %1 , #3"
	:
	:"r" (a), "r" (b));
}
//clr_acc(cp_opa,cp_opb)           //Description : put zero in the destination register Dst (accumulator register).
static inline void clr_acc(int a, int b)
{
    __asm__ __volatile__ ("cp1\tr0, %0, %1 , #4"
	:
	:"r" (a), "r" (b));
}
//move(cp_opa,cp_opb)              //Description : Move the contents of the  data source S to the  destination register Dst.
static inline void move(int a)
{
    __asm__ __volatile__ ("cp1\tr0, %0, r0 , #1"
	:
	:"r" (a));
}
//movei(cp_opa,cp_opb)             //Description : Move an immediate word  into the destination register Dst.
static inline void movei(int a, int b)
{
    __asm__ __volatile__ ("cp1\tr0, %0, %1 , #2"
	:
	:"r" (a), "r" (b));
}
//movei_cp_result(cp_opa,cp_opb)   //Description: Move the content of a source register S into the Cp_result register.
static inline int movei_cp_result(int a)
{ 
    int result;
    __asm__ __volatile__("cp1\t%0, %1,  r0, #7"
		 :"=r" (result)
		 :"r" (a));
    return result;

}


