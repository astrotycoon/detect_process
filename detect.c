#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
/*
 * @函数说明：一个程序检测自身是否已经运行
 * @返回值:
 *		-1: 发生错误，或者pro_name进程不存在
 *		 0: pro_name进程存在
 */
#if 1
int check_process_exist(const char *pro_name)
{
	FILE *fp;
	char cmd[64];
	char buf[64];

	sprintf(cmd, "ps -aux | grep -c %s", pro_name);
	if ((fp = popen(cmd, "r")) == NULL) {
		goto out;
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (atoi(buf) >= 4) {
			pclose(fp);
			return 0;
		}
	}
out:
	if (fp) pclose(fp);	fp = NULL;
	return -1;
}
#else
int check_process_exist(const char *pro_name)
{
	DIR *dp;
	struct dirent *dirp;
	char name[64];
	char line[64];
	FILE *fp;

	if ((dp = opendir("/proc")) == NULL) {
		fprintf(stderr, "open proc error\n");
		goto out;
	}

    while((dirp = readdir(dp)) != NULL) {
        if (dirp->d_type != DT_DIR) continue;
		/* skip dot/dot-dot and self(link) */
        if (strcmp(dirp->d_name, "self") == 0
			|| strcmp(dirp->d_name, ".") == 0
			|| strcmp(dirp->d_name, "..") == 0) continue;

        sprintf(name, "/proc/%s/cmdline", dirp->d_name);
        if ((fp = fopen(name, "r"))) {
            if (fgets(line, sizeof(line), fp) != NULL
				&& strcasestr(line, pro_name)) {  
				/*
			     * d_name就是进程号
                 * 因此，如果存在pro_name，但是进程号却不同，证明进程pro_name已经运行
                 */
				if ((int)getpid() != atoi(dirp->d_name)) {
					fclose(fp); 
					closedir(dp);
					return 0;
				}
			}
        	fclose(fp); 
      	}
    }

out:
	if (dp) closedir(dp); dp = NULL;
	return -1;
}
#endif

int main(int argc, const char *argv[])
{
	if (check_process_exist(program_invocation_short_name) == 0) {
		fprintf(stderr, "Opoos, I have run, so exist.\n");
		exit(EXIT_FAILURE);
	}	
	while (1) { sleep(1); }
	exit(EXIT_SUCCESS);
}
