#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

static char platform_dir[30] = {0};
#define BUFSIZE 1024

void print_version_logs(void)
{
	time_t timep;
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
	printf("--------------------------------------\n");
	printf("|       Tool Version : V1.1.0.2      |\n");
	printf("|       Compile Time : %d/%d/%d    |\n", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday);
	printf("--------------------------------------\n");
}

void usage_print(void)
{
	printf("Error: parameter error, exit\n");
	printf("\nUsage:\n");
	printf("- Make app delta package:\n");
	printf("\t./merge --platform -a oldapp.img newapp.img\n");
	printf("- Make firmware delta package:\n");
	printf("\t./merge --platform -f oldfw.pac newfw.pac\n");
	printf("- Make app&firmware delta package:\n");
	printf("\t./merge --platform -af oldapp.img newapp.img oldfw.pac newfw.pac\n");
	printf("- Make app&firmware delta package:\n");
	printf("\t./merge --platform -b oldapp.img newapp.img fw.bin\n");
	printf("Platform list:\n");
	printf("--8850 --8850BM-C1 --8910 --8910V1.6\n\n");
	return;
}

int calculate_blocksize(char *file1, char *file2)
{
	struct stat file_1_t;
	struct stat file_2_t;
	int file_size1 = 0;
	int file_size2 = 0;
	int blocksize = 0;	//KB
	int block1 = 0;
	int block2 = 0;

	if(stat(file1,&file_1_t)) {
		printf("Error: %s open error\n", file1);
		return -1;
	}
	file_size1 = file_1_t.st_size;

	if(stat(file2,&file_2_t)) {
		printf("Error: %s open error\n", file2);
		return -1;
	}
	file_size2 = file_2_t.st_size;

	if(file_size1 <= 104*1024 && file_size2 <= 104*1024)
		blocksize = 104;
	else{
		for(blocksize=104; blocksize<1000; blocksize+=4) {
			block1 = file_size1/(blocksize*1024) + ((file_size1 - (file_size1/(blocksize*1024))*(blocksize*1024))? 1 : 0);
			block2 = file_size2/(blocksize*1024) + ((file_size2 - (file_size2/(blocksize*1024))*(blocksize*1024))? 1 : 0);
			if(block1 == block2){
				printf("calculate best blocksize : %d, block number : %d\n", blocksize, block1);
				break;
			}
		}
	}
	if(blocksize >= 1000)
	printf("Error: calculate blocksize fail! %d, %d\n", file_size1, file_size2);
	return blocksize;
}

int make_appfota_bin(char *file1, char *file2, int blocksize)
{
	char pdata[512] = {0};
	FILE *fp;
	char* result = NULL;
	char buf[BUFSIZE] = {0};

	unlink("./out/appfota.bin");
	unlink("./out/3.bin");

	sprintf(pdata, "./bin/bsdiff %s %s ./out/appfota.bin %d", file1, file2, blocksize);
	if (system(pdata)) {
		printf("Error: execute \"%s\" fail!\n", pdata);
		goto err_bail;
	}

	memset(pdata, 0, 512);
	sprintf(pdata, "./bin/bspatch %s ./out/3.bin ./out/appfota.bin %d", file1, blocksize);
	if (system(pdata)) {
		printf("Error: execute \"%s\" fail!\n", pdata);
		goto err_bail;
	}

	//check if 3.bin is same as file2
	memset(pdata, 0, 512);
	result = malloc(BUFSIZE);
	if (result == NULL) {
		printf("Error: malloc fail!\n");
		goto err_bail;
	}
	memset(result, 0, BUFSIZE);
	sprintf(pdata, "diff %s %s", file2, "./out/3.bin");
	fp = popen(pdata, "r");
	if (!fp) {
		printf("Error: execute \"%s\" fail!\n", pdata);
		goto err_bail;
	}
	while(fgets(buf, BUFSIZE, fp))
		strcat(result, buf);
	pclose(fp);
	if (*result){
		printf("Error: appfota.bin is wrong\n");
		goto err_bail;
	}

	system("rm -rf ./out/3.bin");
	free(result);result=NULL;
	return 0;

err_bail:
	unlink("./out/appfota.bin");
	unlink("./out/3.bin");
	free(result);result=NULL;
	return -1;
}

int file_sign(char *file)
{
	char pdata[512] = {0};

	if(!memcmp(platform_dir, "bin/8910", strlen(platform_dir))) {
		printf("Info: platform 8910 without sign\n");
		return 0;
	}

	system("chmod 777 out/fw.bin");
	if(!memcmp(platform_dir, "bin/8910V1.6", strlen(platform_dir)))
		sprintf(pdata, "./%s/dtools sign8910 --pn test --pw 12345678 %s ./out/sign.bin", platform_dir, file);
	else
		sprintf(pdata, "./%s/dtools fotasign88xx %s/setting/secp256r1_prv.pem %s/setting/secp256r1_pub.pem %s ./out/sign.bin", platform_dir, platform_dir, platform_dir, file);

	if (system(pdata)) {
		printf("Error: execute \"%s\" fail!\n", pdata);
		return -1;
	}
	system("mv ./out/sign.bin out/fw.bin");

	return 0;
}

int make_fw_bin(char *file1, char *file2)
{
	char pdata[512] = {0};
	char *file = "./out/fw.bin";

	unlink("./out/fw.bin");

	if(access(file1,0) || access(file2,0)){
		printf("Error: %s or %s open error!\n", file1, file2);
		return -1;
	}

	sprintf(pdata, "./%s/dtools fotacreate2 --pac %s,%s,%s/setting/fota.xml ./out/fw.bin -d v", platform_dir, file1, file2, platform_dir);
	if (system(pdata)) {
		printf("Error: execute \"%s\" fail!\n", pdata);
		return -1;
	}

	if (file_sign(file)) {
		printf("Error: file_sign fail!\n");
		return -1;
	}

	return 0;

err_bail:
	unlink("./out/fw.bin");
	return -1;
}

int merge_file(char *file1, char *file2)
{
	char *mem = NULL;
	int fd1 = -1;
	int fd2 = -1;
	int fd_out = -1;
	int read_len = 0;
	int write_len = 0;
	char *appfw_path = "./out/appfw_merge.bin";
	int MAX_SIZE = 2*1024*1024;

	unlink(appfw_path);

	mem = (char *)malloc(MAX_SIZE);
	if(mem == NULL){
		printf("Error: malloc error!\n");
		return -1;
	}
	memset(mem, 0, sizeof(MAX_SIZE));

	fd1 = open(file1, O_RDONLY, 0);
	if(fd1 <= 0){
		printf("Error: open %s failed!\n", file1);
		free(mem);mem=NULL;
		return -1;
	}
	fd2 = open(file2, O_RDONLY, 0);
	if(fd2 <= 0){
		printf("Error: open %s failed!\n", file2);
		free(mem);mem=NULL;
		return -1;
	}
	fd_out = open(appfw_path, O_RDWR | O_CREAT, 0);
	if(fd_out <= 0){
		printf("Error: open %s failed!\n", appfw_path);
		free(mem);mem=NULL;
		return -1;
	}

	read_len = read(fd1, mem, MAX_SIZE);
	if(read_len <= 0){
		printf("Error: read %s file failed.\n", file1);
		free(mem);mem=NULL;
		return -1;
	}
	write_len = write(fd_out, mem, read_len);
	if(write_len != read_len){
		printf("Error: write fd_out failed.\n");
		free(mem);mem=NULL;
		return -1;
	}

	memset(mem, 0, sizeof(MAX_SIZE));

	read_len = read(fd2, mem, MAX_SIZE);
	if(read_len <= 0){
		printf("Error: read %s file failed.\n", file1);
		free(mem);mem=NULL;
		return -1;
	}
	write_len = write(fd_out, mem, read_len);
	if(write_len != read_len){
		printf("Error: write fd_out failed.\n");
		free(mem);mem=NULL;
		return -1;
	}

	printf("merge file SUCCESS! ++++\n");
	close(fd1);
	close(fd2);
	close(fd_out);
	free(mem);mem=NULL;
	return 0;
}

int make_appfw_bin(char *app1, char *app2, 
								char *fw1, char *fw2, int blocksize)
{
	unlink("./out/appfw_merge.bin");

	if(make_appfota_bin(app1, app2, blocksize))
		goto err_bail;

	if(make_fw_bin(fw1, fw2))
		goto err_bail;

	if(merge_file("./out/appfota.bin", "./out/fw.bin"))
		goto err_bail;

	return 0;

err_bail:
	system("rm -rf ./out/*");
	return -1;
}

int make_appfw_bin_ext(char *app1, char *app2, char *fw, int blocksize)
{
	unlink("./out/appfw_merge.bin");

	if(make_appfota_bin(app1, app2, blocksize))
		goto err_bail;

	if(merge_file("./out/appfota.bin", fw))
		goto err_bail;

	return 0;

err_bail:
	system("rm -rf ./out/*");
	return -1;
}

int main(int argc, char *argv[])
{
	int blocksize = 104;    //default 104KB for blocksize

	print_version_logs();

	//platform match
	if(argc == 1)
		goto err_bail;
	if(!memcmp(argv[1], "--8850", strlen(argv[1])))
		strcpy(platform_dir, "bin/8850");
	else if(!memcmp(argv[1], "--8850BM-C1", strlen(argv[1])))
		strcpy(platform_dir, "bin/8850BM-C1");
	else if(!memcmp(argv[1], "--8910", strlen(argv[1])))
		strcpy(platform_dir, "bin/8910");
	else if(!memcmp(argv[1], "--8910V1.6", strlen(argv[1])))
		strcpy(platform_dir, "bin/8910V1.6");
	else
		goto err_bail;

	if(access("./out/", 0)) {
		system("mkdir out/");
	}
	system("rm -rf out/");
	system("mkdir out/");

	//Only app delta package
	if(!memcmp(argv[2], "-a", 2) && (strlen(argv[2]) == 2)){
		if(argc != 5)
			goto err_bail;

		blocksize = calculate_blocksize(argv[3], argv[4]);
		if (blocksize >= 1000 || blocksize <= 100)
			goto err_bail;

		if(make_appfota_bin(argv[3], argv[4], blocksize))
			goto err_bail;
	}

	//Only firmware delta package
	else if(!memcmp(argv[2], "-f", 2) && (strlen(argv[2]) == 2)){
		if(argc != 5)
			goto err_bail;

		if(make_fw_bin(argv[3], argv[4]))
			goto err_bail;
	}

	//App&firmware union delta package
	else if(!memcmp(argv[2], "-af", 3) && (strlen(argv[2]) == 3)){
		if(argc != 7)
			goto err_bail;

		blocksize = calculate_blocksize(argv[3], argv[4]);
		if (blocksize >= 1000 || blocksize <= 100)
			goto err_bail;

		if(make_appfw_bin(argv[3], argv[4], argv[5], argv[6], blocksize))
			goto err_bail;
	}

	//App&firmware union delta package
	else if(!memcmp(argv[2], "-b", 2) && (strlen(argv[2]) == 2)){
		if(argc != 6)
			goto err_bail;

		blocksize = calculate_blocksize(argv[3], argv[4]);
		if (blocksize >= 1000 || blocksize <= 100)
			goto err_bail;

		if(make_appfw_bin_ext(argv[3], argv[4], argv[5], blocksize))
			goto err_bail;
	}

	//Error input param
	else
		goto err_bail;

success_bail:
	system("chmod 777 out/*");
	return 0;

err_bail:
	usage_print();
	return -1;
}