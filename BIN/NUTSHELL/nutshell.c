#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

extern void* gSignalBuffer;

void PrintNutshellLogo() {
	printf(" ___________\n");
	printf("/           \\\n");
	printf("|  GO NUTS  |\n");
	printf("|           |\n");
	printf("|  _______  |\n");
	printf("| | NUTS! | |\n");
	printf("| |50% Off| |\n");
	printf("| |_______| |\tMk1 - VDISK Version\n");
	printf("\\___________/\n\n");
}

/* REMOVE THIS COMMENT NEXT UPDATE - BUG FIXED:
	- If "int logged_in_users = 0;" then when creating a user NUTSHELL would overwrite the elevated "nutshell" user with another user,
	causing lsusr to fail to display the elevated user "nutshell".
	- Fixed to be "int logged_in_users = 1;". Using this method "lsusr" command is aware of the elevated "nutshell" user, and "addusr" command is also aware of the existance
	of the elevated user "nutshell". Using this fix addusr wouldnt overwrite "nutshell" elevated user with another user, which was a serious problem since before changing
	elevated user "nutshell" to index -1, it was using index 0, which meant when creating a user there is no use of it, since it will also be with elevated privellages
	as the elevated user "nutshell" does.
*/
int logged_in_users = 1;
char Users[64][32] = {
	[0] = "nutshell",
};
char CurrentUser[32] = "nutshell";
int CurrentUserI = -1;

void RunTextEditor();

const char* AsciiTable = 
"Dec Hex    Dec Hex    Dec Hex  Dec Hex  Dec Hex  Dec Hex   Dec Hex   Dec Hex\n"
"  0 00 NUL  16 10 DLE  32 20    48 30 0  64 40 @  80 50 P   96 60 `  112 70 p\n"
"  1 01 SOH  17 11 DC1  33 21 !  49 31 1  65 41 A  81 51 Q   97 61 a  113 71 q\n"
"  2 02 STX  18 12 DC2  34 22 \"  50 32 2  66 42 B  82 52 R   98 62 b  114 72 r\n"
"  3 03 ETX  19 13 DC3  35 23 #  51 33 3  67 43 C  83 53 S   99 63 c  115 73 s\n"
"  4 04 EOT  20 14 DC4  36 24 $  52 34 4  68 44 D  84 54 T  100 64 d  116 74 t\n"
"  5 05 ENQ  21 15 NAK  37 25 %  53 35 5  69 45 E  85 55 U  101 65 e  117 75 u\n"
"  6 06 ACK  22 16 SYN  38 26 &  54 36 6  70 46 F  86 56 V  102 66 f  118 76 v\n"
"  7 07 BEL  23 17 ETB  39 27 '  55 37 7  71 47 G  87 57 W  103 67 g  119 77 w\n"
"  8 08 BS   24 18 CAN  40 28 (  56 38 8  72 48 H  88 58 X  104 68 h  120 78 x\n"
"  9 09 HT   25 19 EM   41 29 )  57 39 9  73 49 I  89 59 Y  105 69 i  121 79 y\n"
" 10 0A LF   26 1A SUB  42 2A *  58 3A :  74 4A J  90 5A Z  106 6A j  122 7A z\n"
" 11 0B VT   27 1B ESC  43 2B +  59 3B ;  75 4B K  91 5B [  107 6B k  123 7B {\n"
" 12 0C FF   28 1C FS   44 2C ,  60 3C <  76 4C L  92 5C \\  108 6C l  124 7C |\n"
" 13 0D CR   29 1D GS   45 2D -  61 3D =  77 4D M  93 5D ]  109 6D m  125 7D }\n"
" 14 0E SO   30 1E RS   46 2E .  62 3E >  78 4E N  94 5E ^  110 6E n  126 7E ~\n"
" 15 0F SI   31 1F US   47 2F /  63 3F ?  79 4F O  95 5F _  111 6F o  127 7F DEL\n";

void ShellLoop() {
	char input[2048];

	while (1) {
		printf("%s@nutshell> ", CurrentUser);
		if (!fgets(input, sizeof(input), stdin)) continue;

		int count = strlen(input);
		if (count > 0 && input[count - 1] == '\n')
			input[count - 1] = 0;

		char* cmd = strtok(input, " ");
		if (!cmd) continue;

		if (!strcmp(cmd, "exit")) {
			exit(0);
		} else if (!strcmp(cmd, "help")) {
			printf("Nutshell v1.0.0 - Commands:\n");
			printf("  ascii              Print ASCII table\n");
			printf("  addusr             Add a user to NUTSHELL\n");
			printf("  banner             Show NUTSHELL banner\n");
			printf("  beusr              Become another user\n");
			printf("  calc               Start the calculator interface\n");
			printf("  clear              Clear the TTY view\n");
			printf("  drawbox            Interactive command line draw app\n");
			printf("  exit               Exit the shell\n");
			printf("  echo               Echoes back text\n");
			printf("  elev               Elevate user to NUTSHELL admin privellages\n");
			printf("  help               Show this help text\n");
			printf("  lsusr              List all users\n");
			printf("  poke [E]           Poke memory with a value\n");
			printf("  peek               Read a memory address (up to 512 bytes)\n");
			printf("  version            Current NUTSHELL version\n");
			printf("  whoami             Which user is currently using NUTSHELL\n");
		} else if (!strcmp(cmd, "clear")) {
			printf("\e[2J\e[H");
		} else if (!strcmp(cmd, "ascii")) {
			printf("%s\n", AsciiTable);
		} else if (!strcmp(cmd, "banner")) {
			PrintNutshellLogo();
		} else if (!strcmp(cmd, "calc")) {
			printf("Nutshell not cracked yet...\n\r");
		} else if (!strcmp(cmd, "drawbox")) {
			printf("Nutshell not cracked yet...\n\r");
		} else if (!strcmp(cmd, "echo")) {
			char* echo_text = (char*)&cmd[5];
			printf("%s\n\r", echo_text);
		} else if (!strcmp(cmd, "elev")) {
			printf("Nutshell not cracked yet...\n\r");
		} else if (!strcmp(cmd, "poke")) {
			if (CurrentUserI == 0) {
				printf("Nutshell not cracked yet...\n\r");
			} else {
				printf("Requires elevated privellages...\n\r");
			}
		} else if (!strcmp(cmd, "peek")) {
			printf("Nutshell not cracked yet...\n\r");
		} else if (!strcmp(cmd, "version")) {
			printf("Nutshell version 1.0.0 - Atlas Software & Microsystems Corp.\n\r");
		} else if (!strcmp(cmd, "whoami")) {
			if (CurrentUserI == -1) printf("You are in a ");
			printf("%s", CurrentUser);
			if (CurrentUserI == -1) printf("! :)");
			printf("\n\r");
		} else if (!strcmp(cmd, "addusr")) {
			char* username = strtok(NULL, " ");
			if (username == NULL) continue;
			strcpy(Users[logged_in_users++], username);
		} else if (!strcmp(cmd, "beusr")) {
			char* username = strtok(NULL, " ");
			if (!strcmp(username, "nutshell")) {
				printf("Cannot be \"nutshell\", to switch to \"nutshell\" use the \"elev\" command\n\r");
				username[0] = 0;
			}
			int idx = -1;
			for (int i = 0; i < logged_in_users; i++) {
				if (!strcmp(Users[i], username)) {
					CurrentUserI = i;
					int last = 0;
					for (int j = 0; username[j] != 0 && j < 32; j++) {
						CurrentUser[j] = username[j];
						last = j;
					}
					last++;
					for (int j = last; j < 32; j++) {
						CurrentUser[j] = 0;
					}
				}
			}
		} else if (!strcmp(cmd, "lsusr")) {
			for (int i = 0; i < logged_in_users; i++) {
				printf("%s\n", Users[i]);
			}
		} else if (!strcmp(cmd, "nutshell::read_signal_buf")) {
			printf("Signal buffer: %d\n\r", *(int*)gSignalBuffer);
		} else if (!strcmp(cmd, "nutshell:be_elevated_user")) {
			CurrentUserI = -1;
			char nutshell_name[32] = {0};
			strncpy(nutshell_name, "nutshell", strlen("nutshell"));
			for (int i = 0; i < 32; i++) {
				CurrentUser[i] = nutshell_name[i];
			}
		}

		else {
			printf("Unknown command: %s\n", cmd);
		}
	}
}

int main(int argc, char** argv, char** envp) {
	(void)argc; (void)argv; (void)envp;

	printf("SIGNAL BUFFER AT %p\n\r", gSignalBuffer);
	printf("SIGNAL0 RECIEVED: %d\n\r", *(int*)gSignalBuffer);

	PrintNutshellLogo();
	ShellLoop();
	return 0;
}
