#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

extern void* gSignalBuffer;

void PrintNutshellLogo();
void RunTextEditor();
void RestoreUserInfoPastElev();

void cmd_help();
void cmd_clear();
void cmd_ascii();
void cmd_banner();
void cmd_calc();
void cmd_drawbox();
void cmd_echo(char* args);
void cmd_elev();
void cmd_poke();
void cmd_peek();
void cmd_version();
void cmd_whoami();
void cmd_addusr(char* username);
void cmd_beusr(char* username);
void cmd_lsusr();
void cmd_read_signal_buf();
void cmd_be_elevated_user();

void PrintNutshellLogo() {
	 printf(" ___________\n");
	 printf("/           \\\n");
	 printf("| NUTSHELL! |\n");
	 printf("|           |\n");
	 printf("|  _______  |\n");
	 printf("| | NUTS! | |\n");
	 printf("| |50%% Off| |\n");
	printf("\\___________/\n\n");
}


int logged_in_users = 1;
char Users[64][32] = { [0] = "nutshell" };
char CurrentUser[32] = "nutshell";
int CurrentUserI = 0;

int EnabledElevatedUserByCmd = 0;
int ElevatedUserInfoCurrentUserI = 0;
char ElevatedUserInfoCurrentUser[32] = {0};

void RunTextEditor();

const char AsciiTable[] = 
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
		if (count > 0 && input[count - 1] == '\n') input[count - 1] = 0;

		char* cmd = strtok(input, " ");
		if (!cmd) continue;

		if (!strcmp(cmd, "exit")) {
			if (EnabledElevatedUserByCmd)
				RestoreUserInfoPastElev();
			else
				exit(0);
		} else if (!strcmp(cmd, "help")) {
			cmd_help();
		} else if (!strcmp(cmd, "clear")) {
			cmd_clear();
		} else if (!strcmp(cmd, "ascii")) {
			cmd_ascii();
		} else if (!strcmp(cmd, "banner")) {
			cmd_banner();
		} else if (!strcmp(cmd, "calc")) {
			cmd_calc();
		} else if (!strcmp(cmd, "drawbox")) {
			cmd_drawbox();
		} else if (!strcmp(cmd, "echo")) {
			cmd_echo(strtok(NULL, ""));
		} else if (!strcmp(cmd, "elev")) {
			cmd_elev();
		} else if (!strcmp(cmd, "poke")) {
			cmd_poke();
		} else if (!strcmp(cmd, "peek")) {
			cmd_peek();
		} else if (!strcmp(cmd, "version")) {
			cmd_version();
		} else if (!strcmp(cmd, "whoami")) {
			cmd_whoami();
		} else if (!strcmp(cmd, "addusr")) {
			char* username = strtok(NULL, " ");
			cmd_addusr(username);
		} else if (!strcmp(cmd, "beusr")) {
			char* username = strtok(NULL, " ");
			cmd_beusr(username);
		} else if (!strcmp(cmd, "lsusr")) {
			cmd_lsusr();
		} else if (!strcmp(cmd, "nutshell::read_signal_buf")) {
			cmd_read_signal_buf();
		} else if (!strcmp(cmd, "nutshell::be_elevated_user")) {
			cmd_be_elevated_user();
		} else {
			printf("Unknown command: %s\n", cmd);
		}
	}
}

int main(int argc, char** argv, char** envp) {
	(void)argc; (void)argv; (void)envp;

	PrintNutshellLogo();
	ShellLoop();
	return 0;
}

void cmd_help() {
	printf("NUTSHELL v1.0.0 - Commands:\n");
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
}

void cmd_clear() {
	printf("\x1b[?25l\e[2J\e[H\x1b[?25h");
}

void cmd_ascii() {
	printf("%s\n", AsciiTable);
}

void cmd_banner() {
	PrintNutshellLogo();
}

void cmd_calc() {
	printf("Nutshell was not cracked yet...\n\r");
}

void cmd_drawbox() {
	printf("Nutshell was not cracked yet...\n\r");
}

void cmd_echo(char* args) {
	if (args) printf("%s\n\r", args);
}

void cmd_elev() {
	CurrentUserI = 0;
	EnabledElevatedUserByCmd = 1;
	strncpy(ElevatedUserInfoCurrentUser, CurrentUser, 32);
	ElevatedUserInfoCurrentUserI = CurrentUserI;
	strcpy(CurrentUser, "nutshell");
}

void cmd_poke() {
	if (CurrentUserI == 0)
		printf("Nutshell was not cracked yet...\n\r");
	else
		printf("Requires elevated privellages...\n\r");
}

void cmd_peek() {
	printf("Nutshell was not cracked yet...\n\r");
}

void cmd_version() {
	printf("NUTSHELL version 1.0.0 - Atlas Software & Microsystems Corp.\n\r");
}

void cmd_whoami() {
	if (CurrentUserI == -1) printf("You are in a ");
	printf("%s", CurrentUser);
	if (CurrentUserI == -1) printf("! :)");
	printf("\n\r");
}

void cmd_addusr(char* username) {
	if (!username) {
		printf("Usage: addusr <USERNAME>\n\r");
		return;
	}
	for (int i = 0; i < logged_in_users; i++) {
		if (!strcmp(Users[i], username)) return;
	}
	if (logged_in_users < 64)
		strcpy(Users[logged_in_users++], username);
}

void cmd_beusr(char* username) {
	if (!username) return;
	if (!strcmp(username, "nutshell")) {
		printf("Cannot be \"nutshell\", to switch to \"nutshell\" use the \"elev\" command\n\r");
		return;
	}
	for (int i = 0; i < logged_in_users; i++) {
		if (!strcmp(Users[i], username)) {
			CurrentUserI = i;
			strncpy(CurrentUser, username, 31);
			CurrentUser[31] = 0;
			return;
		}
	}
}

void cmd_lsusr() {
	for (int i = 0; i < logged_in_users; i++) {
		printf("%s\n", Users[i]);
	}
}

void cmd_read_signal_buf() {
	printf("Signal buffer: %d\n\r", *(int*)gSignalBuffer);
}

void cmd_be_elevated_user() {
	CurrentUserI = 0;
	strcpy(CurrentUser, "nutshell");
}

void RestoreUserInfoPastElev() {
	cmd_beusr(ElevatedUserInfoCurrentUser);
}
