#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	long long ans=1ll*a*b;
	return (ans>>16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
	/* Dividing two 64-bit integers needs the support of another library
	 * `libgcc', other than newlib. It is a dirty work to port `libgcc'
	 * to NEMU. In fact, it is unnecessary to perform a "64/64" division
	 * here. A "64/32" division is enough.1
	 *
	 * To perform a "64/32" division, you can use the x86 instruction
	 * `div' or `idiv' by inline assembly. We provide a template for you
	 * to prevent you from uncessary details.
	 *
	 *     asm volatile ("??? %2" : "=a"(???), "=d"(???) : "r"(???), "a"(???), "d"(???));
	 *
	 * If you want to use the template above, you should fill the "???"
	 * correctly. For more information, please read the i386 manual for
	 * division instructions, and search the Internet about "inline assembly".
	 * It is OK not to use the template above, but you should figure
	 * out another way to perform the division.
	 */
	
	/*as we konw , FLOAT sotre as int, you can just think it as int, but when it comes to calculate, there seem to be some questions.
	  but we can use int'/', so There is a solution which make full use of int'/'*/
	int pon=1;
	if(a<0)
	{
		pon=pon*(-1);
		a=a*(-1);
	}
	if(b<0)
	{
		pon=pon*(-1);
		b=b*(-1);
	}
	/*just get rid of the peoblem of negative numbers
	  so there are two positive numbers, much easiser!*/
	
	/*there are two situations
	    1.a>=b, a/b>0, and we should let a/b go to before 16bits(FLOAT you konw)
	    2.a<b , a/b=0, it should stay in the last 16bits
	  so whatever, we must <<16*/
	int ans=a/b;
	int temp=a%b;
	int i;
	for(i=0;i<16;i++)
	{
		temp<<1;
		ans<<1;
		if(temp>=b)
		{
			temp-=b;
			ans++;
		}	
	}
	return ans*pon;
}

FLOAT f2F(float a) {
	/* You should figure out how to convert `a' into FLOAT without
	 * introducing x87 floating point instructions. Else you can
	 * not run this code in NEMU before implementing x87 floating
	 * point instructions, which is contrary to our expectation.
	 *
	 * Hint: The bit representation of `a' is already on the
	 * stack. How do you retrieve it to another variable without
	 * performing arithmetic operations on it directly?
	 */

	nemu_assert(0);
	return 0;
}

FLOAT Fabs(FLOAT a) {
	nemu_assert(0);
	return 0;
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

