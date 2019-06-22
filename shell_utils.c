

void clearScreen(){
    printf("\033[H\033[J");
}

char * trim(char *s){
    char *p = s;
    int len = strlen(p);

    while(isspace(p[len-1]))
        p[--len] = '\0';
    while(*p && isspace(*p))
        p++, len--;

    return p;
}

void getPrompt(char *prompt){
    char curr_dir[CWD_LEN];
    getcwd(curr_dir, CWD_LEN);
    char * user = getenv("USER");
    char result[CWD_LEN] = "";
    strcat(result, "[");
    strcat(result, user);
    strcat(result, "]");
    strcat(result, "[");
    strcat(result, curr_dir);
    strcat(result, "]\n$$ ");
    strcpy(prompt, result);
}

int getInput(char * ip){
    char prompt[CWD_LEN];
    getPrompt(prompt);
    char * buff = readline(prompt);
    if(strlen(buff) != 0){
        add_history(buff);
        strcpy(ip, buff);
        return 1;
    }
    return 0;
}
