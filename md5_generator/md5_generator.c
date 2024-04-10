#include "md5.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <sys/stat.h>
#include <dirent.h>
#endif

#define PROGRAM_NAME "md5_generator"
#define PROGRAM_VERSION "version: 1.0\nauthor: unagi"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define MAX_PATH_LENGTH 2048

typedef enum
{
	ARG_HELP,
	ARG_VERSION,
	ARG_PATH,
	ARG_UNKNOWN
} ArgumentType;

typedef enum
{
	PATH_REGULARFILE,
	PATH_DIRECTORY,
	PATH_OTHER
} PathType;

typedef struct
{
	const char *option;
	ArgumentType type;
} OptionMap;

OptionMap options[] = {
	{"-h", ARG_HELP},
	{"--help", ARG_HELP},
	{"-v", ARG_VERSION},
	{"--version", ARG_VERSION},
	{"-p", ARG_PATH},
	{"-path", ARG_PATH},
	{NULL, ARG_UNKNOWN}};

void print_help()
{
	printf("Usage: %s [OPTIONS]\n\n", PROGRAM_NAME);
	printf("Description:\n");
	printf("  This program generates md5 digest for a signal file or all files in a folder.\n\n");
	printf("Options:\n");
	printf("  -h, --help                Display this help message\n");
	printf("  -v, --version             Display version information\n");
	printf("  -p, --path <path>     Specify the path of the file or folder\n");
}

ArgumentType get_argument_type(const char *arg)
{
	for (int i = 0; options[i].option != NULL; ++i)
	{
		if (strcmp(arg, options[i].option) == 0)
		{
			return options[i].type;
		}
	}
	return ARG_UNKNOWN;
}

int file_md5(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (!file)
	{
		fprintf(stderr, "Failed to open file %s\n", filename);
		return 1;
	}

	MD5_CTX ctx;
	unsigned char digest[MD5_DIGEST_SIZE];
	unsigned char buffer[1024];
	size_t bytesRead;

	md5_init(&ctx);

	while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) != 0)
	{
		md5_update(&ctx, buffer, bytesRead);
	}

	md5_final(digest, &ctx);

	fclose(file);

	printf("MD5 digest of file %s: ", filename);
	for (int i = 0; i < MD5_DIGEST_SIZE; i++)
	{
		printf("%02x", digest[i]);
	}
	printf("\n");
	return 0;
}

int get_file_type(const char *path)
{
#ifdef _WIN32
	DWORD attrs = GetFileAttributes(path);
	if (attrs != INVALID_FILE_ATTRIBUTES)
	{
		if (attrs & FILE_ATTRIBUTE_DIRECTORY)
		{
			return PATH_DIRECTORY;
		}
		else
		{
			return PATH_REGULARFILE;
		}
	}
	else
	{
		perror("GetFileAttributes");
		return -1;
	}
#elif __linux__
	struct stat st;
	if (stat(path, &st) == 0)
	{
		if (S_ISREG(st.st_mode))
		{
			return PATH_REGULARFILE;
		}
		else if (S_ISDIR(st.st_mode))
		{
			return PATH_DIRECTORY;
		}
		else
		{
			return PATH_OTHER;
		}
	}
	else
	{
		perror("stat");
		return -1;
	}
#endif
}

int traverse_directory_files(const char *path)
{
#ifdef _WIN32
	char search_path[MAX_PATH_LENGTH];
	sprintf(search_path, "%s\\*", path);

	WIN32_FIND_DATA find_data;
	HANDLE handle = FindFirstFile(search_path, &find_data);
	if (handle == INVALID_HANDLE_VALUE)
	{
		perror("FindFirstFile");
		exit(EXIT_FAILURE);
	}

	do
	{
		if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0)
			continue;
		char file_path[MAX_PATH_LENGTH];
		sprintf(file_path, "%s\\%s", path, find_data.cFileName);

		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			traverse_directory_files(file_path);
		}
		else
		{
			file_md5(file_path);
			// printf("File: %s\n", find_data.cFileName);
		}

	} while (FindNextFile(handle, &find_data) != 0);

	FindClose(handle);
#elif __linux__
	DIR *dir = opendir(path);
	if (dir == NULL)
	{
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		char file_path[MAX_PATH_LENGTH];
		snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);

		if (entry->d_type == DT_DIR)
		{
			traverse_directory_files(file_path);
		}
		else
		{
			file_md5(file_path);
			// printf("File: %s\n", entry->d_name);
		}
	}

	closedir(dir);
#endif
}

int all_files_md5(const char *path)
{
	int type;
	if ((type = get_file_type(path)) < 0)
	{
		fprintf(stderr, "Error: Invalid path.\n");
		exit(EXIT_FAILURE);
	}
	if (type == PATH_DIRECTORY)
	{
		traverse_directory_files(path);
	}
	else if (type == PATH_REGULARFILE)
	{
		file_md5(path);
	}
	else
	{
		fprintf(stderr, "Error: Unknown file type.\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}

void handle_argument(ArgumentType arg_type, const char *arg_value)
{
	switch (arg_type)
	{
	case ARG_HELP:
		print_help();
		exit(EXIT_SUCCESS);
	case ARG_VERSION:
		printf("%s\n%s\n", PROGRAM_NAME, PROGRAM_VERSION);
		exit(EXIT_SUCCESS);
	case ARG_PATH:
		all_files_md5(arg_value);
		exit(EXIT_SUCCESS);
	default:
		fprintf(stderr, "Error: Unknown option.\n");
		print_help();
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Error: No arguments provided.\n");
		print_help();
		return EXIT_FAILURE;
	}

	ArgumentType arg_type = get_argument_type(argv[1]);
	if (arg_type != ARG_UNKNOWN)
	{
		if (arg_type == ARG_PATH)
		{
			if (argc == 3)
			{
				handle_argument(arg_type, argv[2]);
			}
			else
			{
				print_help();
				return EXIT_FAILURE;
			}
		}
		else
		{
			if (argc != 2)
			{
				print_help();
				return EXIT_FAILURE;
			}
			handle_argument(arg_type, NULL);
		}
	}
	else
	{
		all_files_md5(argv[1]);
	}

	return EXIT_SUCCESS;
}