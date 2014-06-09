/* change governors on the fly */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for strtok() */
#include <unistd.h> /* for sysconf() */

#define MAX_GOVERNORS 5
#define VERSION "1.00"

char *f_current_gov_start = "/sys/devices/system/cpu/cpu";
char *f_current_gov_end = "/cpufreq/scaling_governor";
char *f_avail_govs = "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors";
char *avail_govs[MAX_GOVERNORS] = {0};

void show_current_gov(int coreidx)
{
	FILE *f;
	char line[256] = {0};
	char gov_name[256] = {0}; /* collect full name here */
	char stridx[2] = {0}; /* strcat() needs at least 2 chars */ 
	char c = coreidx + 48; /* convert int to ascii here */
	stridx[0] = c;

	strcat(gov_name, f_current_gov_start);
	strcat(gov_name, stridx);
	strcat(gov_name, f_current_gov_end);

	//printf("gov_name: %s \n", gov_name);

	f = fopen(gov_name, "r");
	if (!f)
	{
		printf("<can't find current governor. cpufreq seems to be"
			" disabled in kernel>\n");
		exit(1);
	}

	if (fgets(line, sizeof(line), f) == NULL)
	{
		printf("<can't read list of available governors>\n");
		exit(1);
	}
	fclose(f);
	printf("[core %d governor:] %s",coreidx, line);
}

void set_current_gov(int coreidx, int govidx)
{
	FILE *f;
	char gov_name[256] = {0}; /* collect full name here */
	char stridx[2] = {0}; /* strcat() needs at least 2 chars */ 
	char c = coreidx + 48; /* convert int to ascii here */
	stridx[0] = c;

	strcat(gov_name, f_current_gov_start);
	strcat(gov_name, stridx);
	strcat(gov_name, f_current_gov_end);
	f = fopen(gov_name, "w");
	if (!f)
	{
		printf("<can't find current governor>\n");
		exit(1);
	}
	
	fputs(avail_govs[govidx], f);
	fclose(f); 
}

int main()
{
	FILE *f;
	char line[256];
	char *p_line;
	int num_govs;
	int num_cores;
	int cnt = 0;
	int i;
	int idx; /* index which user enters from keyboard */

	/* get number of cores at runtime */
	num_cores = sysconf(_SC_NPROCESSORS_ONLN); 
	printf("[cores online   :] %d \n", num_cores);

	/* show governors for every core */
	for (i = 0; i < num_cores; i++)
		show_current_gov(i);

	/* show available governors in system */
	f = fopen(f_avail_govs, "r");
	if (!f)
	{
		printf("<can't find list of available governors>\n");
		exit(1);
	}
	if (fgets(line, sizeof(line), f) == NULL)
	{
		printf("<can't read list of available governors>\n");
		exit(1);
	}
	fclose(f);
	printf("[avail governors:] %s", line);

	/* parse line of avail governors and store them in array */
	p_line = line;
	while ((avail_govs[cnt] = strtok(p_line, " ")) != NULL)
	{
		//printf("cnt: %d , word: %s\n", cnt, avail_govs[cnt]);
		/* pass string to strtok only first time, then pass NULL */
		p_line = NULL;
		cnt++;
	}
	num_govs = cnt;

	printf("---------- list of governors ---------- \n");
	for (cnt = 0; cnt < num_govs; cnt++)
	{
		/* check governors list for sanity and drop 0xA */
		if (*avail_govs[cnt] == 0xA)
		{
			num_govs--;
			break;
		}
		printf("press [#%d] to select \"%s\" \n",
			 cnt+1, avail_govs[cnt]);
	}
		printf("press [#0] to exit\n");

	/* get user input */
	scanf("%d", &idx);
	if (idx < 0 || idx > num_govs)
	{
		printf("<wrong value entered. exit>\n");
		exit(1);
	}
	else if (idx == 0)
	{
		exit(0);
	}
	idx--; /* decrease entered value to use as array index */

	/* switch governors for every core */
	for (i = 0; i < num_cores; i++)
		set_current_gov(i, idx);

	/* show governors for every core */
	for (i = 0; i < num_cores; i++)
		show_current_gov(i);

	return 0;
}
