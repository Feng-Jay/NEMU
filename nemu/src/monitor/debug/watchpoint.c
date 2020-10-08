#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

WP* new_wp()
{
	WP *hp,*fp;
	fp=free_;
	hp=head;
	if(free_->next!=NULL)
	free_=free_->next;
	else 
	{
		printf("Storage is full!\n");
		assert(0);
		}
	fp->next=NULL;
	if(hp==NULL)
	{
		head=fp;
		hp=head;
	}
	else 
	{
		while(hp->next!=NULL)
		{
			hp=hp->next;
		}
		hp->next=fp;
	}
	return fp;
	
}

void free_wp(WP* wp)
{
	WP *hp,*fp;
	fp=free_;
	hp=head;
	if(hp==NULL)
	assert(0);
	if(head->NO==wp->NO)
	head=head->next;
	else 
	{
		while(hp->next!=NULL&&hp->next->NO!=wp->NO)
			hp=hp->next;
		if(hp->next->NO==wp->NO)
			hp->next=hp->next->next;
		else
 		assert(0);
	}

	 if(fp==NULL)
        {
                free_=wp;
                fp=free_;
        }
        else
        {
                while(fp->next!=NULL)
                fp=fp->next;
                fp->next=wp;
        }

	wp->next=NULL;
	wp->val=0;
	wp->expr[0]='\0';
}
bool check_wp()
{
	WP* hp;
	hp=head;
	bool success;
	bool outcome=true;
	while(hp!=NULL)
	{
		uint32_t now_value = expr(hp->expr,&success);
		if(!success)
		assert(1);
		if(now_value!=hp->val)
		{
			outcome=false;
			printf("Watchpoint %d:%s's value change from %d to %d \n",(hp->NO)+1,hp->expr,hp->val,now_value);
			hp->val=now_value;
		}
		hp=hp->next;
		
	}	
	return outcome;
}
void delete_wp(int no)
{
	WP* hp;
	hp=&wp_pool[no-1];
	free_wp(hp);
}
void info_wp()
{
	WP* hp;
	hp=head;
	while(hp!=NULL)
	{
		printf("Watchpoint %d: %s = %d \n",(hp->NO)+1,hp->expr,hp->val);
		hp=hp->next;
	}
}
/* TODO: Implement the functionality of watchpoint */


