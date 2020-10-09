#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ,NEQ,AND,OR,HEX,DEX,NEG,POI,REG

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

        {"\\b0[xX][0-9a-fA-F]+\\b",HEX,0},                       //HEX
        {"\\b[0-9]+\\b",DEX,0},                               //DEX
        {"\\$[a-zA-Z]+",REG,0},                          //register
        {"!=",NEQ,3},                                   //not equal
	{"!",'!',6},					//not
	{"\\*",'*',5},                                  //MUL
	{"/",'/',5},                                    //div
	{" +",NOTYPE,0},				//spaces  
	{"\\+",'+',4},					//plus
	{"-",'-',4},                                  //sub
        {"&&",AND,2},                                   //AND
        {"==",EQ,3},                                    //equal
        {"\\|\\|",OR,1},                                //or
	{"\\(",'(',7},					//front
	{"\\)",')',7},					//post
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				char* REGS=e+position+1;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE: break;
					case REG:
					tokens[nr_token].type=rules[i].token_type;
					tokens[nr_token].priority=rules[i].priority;
					strncpy(tokens[nr_token].str,REGS,substr_len-1);
					tokens[nr_token].str[substr_len-1]='\0';
					nr_token++;
					break;
					default: 
					tokens[nr_token].type=rules[i].token_type;
					tokens[nr_token].priority=rules[i].priority;
					strncpy(tokens[nr_token].str,substr_start,substr_len);
					tokens[nr_token].str[substr_len]='\0';
					nr_token++;
					break;

				}
				position += substr_len;
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

bool check_parentheses(int p,int q){

	int i;
	int tag=0;
	if(tokens[p].type!='('||tokens[q].type!=')')
	return false;
	for(i=p;i<=q;i++){
		if(tokens[i].type=='(')
		tag++;
		else if(tokens[i].type==')')
		tag--;
		if(tag==0&&i<q)
		return false;
	}
	if(tag!=0)
	return false;
	return true;
}

int dominant_operator(int p,int q){
	
	int min_priority=23;
	int operator=p;
	int i;
	int countt=0;
	for(i=p;i<=q;i++){
		if(tokens[i].type==HEX||tokens[i].type==DEX||tokens[i].type==REG)
		continue;
		if(tokens[i].type=='(')
		countt++;
		if(tokens[i].type==')')
		countt--;
		if(countt!=0)
		continue;
		if(tokens[i].priority<=min_priority)
		{
			min_priority=tokens[i].priority;
			operator=i;
		}
		
	}
	return operator;
}

uint32_t eval(int p,int q){

	if(p>q){
		Assert(p>q,"HHHH\n");
		return 0;
	}

	if(p==q){
		uint32_t outcome=0;
		if(tokens[p].type==HEX)
		sscanf(tokens[p].str,"%x",&outcome);
		if(tokens[p].type==DEX)
		sscanf(tokens[p].str,"%d",&outcome);
		if(tokens[p].type==REG){
			if(strlen(tokens[p].str)==3){
				int i;
				for(i=0;i<8;i++){
					if(strcmp(tokens[p].str,regsl[i])==0)
					break;
				}
				outcome=cpu.gpr[i]._32;
			}
			
		  	else if (strlen(tokens[p].str)==2){
				if(tokens[p].str[1]=='l'||tokens[p].str[1]=='h'){
					int i;
					for(i=0;i<8;i++){
					if(strcmp(tokens[p].str,regsb[i])==0)
					break;
					}
					outcome=cpu.gpr[i]._8[i>>2];
				}
				else if(tokens[p].str[1]=='x'||tokens[p].str[1]=='p'||tokens[p].str[1]=='i'){
					int i;
					for(i=0;i<8;i++){
					if(strcmp(tokens[p].str,regsw[i])==0)
					break;}
					outcome =cpu.gpr[i]._16;
				}
			}
		}
		return outcome;
		
	}
	
	else if(check_parentheses(p,q)==true){

		return eval(p+1,q-1);

	}

	
	else {
		
		int op=dominant_operator(p,q);
		if(op==p||tokens[op].type==NEG||tokens[op].type==POI||tokens[op].type=='!'){
			uint32_t exp=eval(p+1,q);
			switch(tokens[op].type){
			case POI:return swaddr_read(exp,4);
			case NEG:return -(exp);
			case '!':return !(exp);
			
			}
		}
		uint32_t  exp1=eval(p,op-1);
		uint32_t  exp2=eval(op+1,q);
		switch(tokens[op].type){
			case '+':return exp1+exp2;
			case '-':return exp1-exp2;
			case '*':return exp1*exp2;
			case '/':return exp1/exp2;
			case OR :return exp1||exp2;
			case AND:return exp1&&exp2;
			case EQ :return exp1==exp2;
			case NEQ:return exp1!=exp2;
			default: break;
		}
	}
	assert(1);
	return -1;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	int i;
	for(i=0;i<nr_token;i++){
		if(tokens[i].type=='-'&&(i==0||(tokens[i-1].type!=DEX&&tokens[i-1].type!=HEX&&tokens[i-1].type!=')'))){
			tokens[i].type=NEG;
			tokens[i].priority=6;	
		}
		if(tokens[i].type=='*'&&(i==0||(tokens[i-1].type!=DEX&&tokens[i-1].type!=HEX&&tokens[i-1].type!=')'))){
			tokens[i].type=POI;
			tokens[i].priority=6;
		}
	}
	*success=true;
	return eval(0,nr_token-1);

}

