#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);
/*git try234*/

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_si(char *args) {
	char* configg=strtok(NULL," ");
	if(configg==NULL)
	{
		cpu_exec(1);
		return 0; 	}
	int exe_times=0;
	exe_times=atoi(configg);
	if(exe_times<0)
	{
		printf("Input a wrong argument! \n");
		return 0;
				}
	else 
	cpu_exec(exe_times);	
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_info(char *args) {
	char* configg=strtok(NULL," ");
	int i=0;
	if (strcmp(configg,"r")==0)
	{
		for(i=0;i<8;i++)
		{
			printf("%s: 0x%x %d \n",regsl[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
		}
		return 1;
	}
	else if(strcmp(configg,"w")==0)
	{
		info_wp();
		return 1;
	}
	else 
	{
		printf("Input agrument wrong!\n");
		return 0;
		}
}

static int cmd_storage(char *args)
{
		char* first_con=strtok(NULL," ");
		char* second_con=strtok(NULL," ");
		int rrange=0;
		swaddr_t llocal;
		sscanf(first_con,"%d",&rrange);
		sscanf(second_con,"%x",&llocal);
		int i=0;
		for(i=0;i<rrange;i++)
	{
		printf("0x%08x: ",llocal+i*4);
		int data;
		data=swaddr_read(llocal+i*4,4);
		int j=0;
		for(j=0;j<4;j++){
		printf("0x%02x ",data&0xff);
		data=data>>8;	}	

		printf("\n");
		
					}
	
		return 1;
			
} 

static int cmd_p(char* agrs)
{
	uint32_t outtcome;
	bool success;
	outtcome=expr(agrs,&success);
	if(success)
	printf("Hex:0x%x DEC:%d\n",outtcome,outtcome);
	else 
	assert(0);
	return 0;
}

static int cmd_w(char* agrs)
{
	WP* p;
	bool success;
	p=new_wp();
	printf("Watchpoint %d: %s set",(p->NO)+1,agrs);
	p->val=expr(agrs,&success);
	if(!success)
	Assert(1,"Wrong cmd format\n");
	strcpy(p->expr,agrs);
	printf("Now value is %d\n",p->val);
	return 0;
	
}

static int cmd_d(char* agrs)
{
	int num;
	sscanf(agrs,"%d",&num);
	delete_wp(num);
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
        { "si","exe the following N cmds",cmd_si},
	{"info","r to show the status of registers||w to show watchpoint",cmd_info},
	{"x","Scan the storage!",cmd_storage},
	{"p","calculate the value of expressions",cmd_p},
	{"w","Watch the value of expression",cmd_w},
	{"d","Delete the No.N watchpoint",cmd_d},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
